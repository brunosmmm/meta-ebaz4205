#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/delay.h>

#define DRIVER_NAME "kswfloppy"

#define _SIG_MOTEA 0
#define _SIG_DRVSB 1
#define _SIG_DRVSA 2
#define _SIG_MOTEB 3
#define _SIG_DIR 4
#define _SIG_STEP 5
#define _SIG_WPT 6
#define _SIG_TRK00 7
#define _SIG_SIDE1 8
#define _SIG_WDATE 9
#define _SIG_WGATE 10
#define _SIG_REDWC 11
#define _SIG_INDEX 12
#define _SIG_DSKCHG 13
#define _SIG_RDATA 14

#define FLOPPY_STEP_INWARDS 0
#define FLOPPY_STEP_OUTWARDS 1

#define FLOPPY_TRACKS 80

#define FLOPPY_FLAG_HOMED 0x1

// pulsecap
#define PULSECAP_STATE_IDLE 0
#define PULSECAP_STATE_RUN 1
#define PULSECAP_STATE_DONE 2

struct pulsecap_data;
extern int pulsecap_start_capture(struct pulsecap_data* pcap,
                                  unsigned int edge_count);
extern int pulsecap_cancel_capture(struct pulsecap_data* pcap);
extern int pulsecap_get_state(struct pulsecap_data* pcap);
extern int pulsecap_read_edges(struct pulsecap_data* pcap,
                               unsigned int count,
                               unsigned int* dest);
extern int pulsecap_get_max_edges(struct pulsecap_data* pcap);

struct aximux_device;
extern void aximux_set_src(struct aximux_device* dev, unsigned int port,
                           unsigned int source);

const unsigned char FLOPPY_SIGNAL_DIRECTIONS[15] = {
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_IN,
    GPIOD_IN,
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_OUT_LOW,
    GPIOD_IN,
    GPIOD_IN,
    GPIOD_IN,
    GPIOD_IN};

struct kswfloppy_driver_instance {

};

struct kswfloppy_data {
  struct gpio_desc* floppy_gpios[15];
  unsigned int cur_track;
  unsigned int flags;
  unsigned char dump[32000];
  struct pulsecap_data* pcap_inst;
  struct aximux_device* mux_inst;
  unsigned int capture_pin;
};

//static struct kswfloppy_driver_instance driver_data;

static inline void assert_signal(struct kswfloppy_data* handler,
                                 unsigned char signal) {

  gpiod_set_value(handler->floppy_gpios[signal], 1);
}

static inline void deassert_signal(struct kswfloppy_data* handler,
                                   unsigned char signal) {

  gpiod_set_value(handler->floppy_gpios[signal], 0);
}

static inline int read_signal(struct kswfloppy_data* handler,
                              unsigned char signal,
                              unsigned int* state) {
  *state = gpiod_get_value(handler->floppy_gpios[signal]);
  return 0;
}

static void deassert_all(struct kswfloppy_data* handler) {
  unsigned int ii =0;
  for (ii=0;ii<15;ii++) {
    deassert_signal(handler, ii);
  }
  udelay(1);
}

static inline void _floppy_en(struct kswfloppy_data* handler) {
  assert_signal(handler, _SIG_DRVSB);
  udelay(1);
}

static inline void _floppy_dis(struct kswfloppy_data* handler) {
  deassert_signal(handler, _SIG_DRVSB);
  udelay(1);
}

static inline void _spin_on(struct kswfloppy_data* handler) {
  assert_signal(handler, _SIG_MOTEB);
  udelay(1);
}

static inline void _spin_off(struct kswfloppy_data* handler) {
  deassert_signal(handler, _SIG_MOTEB);
  udelay(1);
}

static void floppy_step(struct kswfloppy_data* handler, unsigned char direction) {
  // enable
  //_floppy_en(handler);

  deassert_signal(handler, _SIG_STEP);
  udelay(1);
  if (direction == FLOPPY_STEP_INWARDS) {
    assert_signal(handler, _SIG_DIR);
    if (handler->cur_track < FLOPPY_TRACKS) {
      handler->cur_track++;
    }
  } else {
    deassert_signal(handler, _SIG_DIR);
    if (handler->cur_track > 0) {
      handler->cur_track--;
    }
  }
  // direction setup time
  udelay(1);

  // step
  assert_signal(handler, _SIG_STEP);
  udelay(8);
  deassert_signal(handler, _SIG_STEP);

  // disable
  //_floppy_dis(handler);

  // rate-limit stepping
  usleep_range(3000, 20000);
}

static void floppy_home(struct kswfloppy_data* handler) {
  unsigned int state = 0, stuck_counter = 100;
  _floppy_en(handler);
  while(!read_signal(handler, _SIG_TRK00, &state)) {
    if (state) {
      // in track 0
      handler->cur_track = 0;
      handler->flags |= FLOPPY_FLAG_HOMED;
      break;
    } else {
      floppy_step(handler, FLOPPY_STEP_OUTWARDS);
    }
    if (stuck_counter) {
      stuck_counter--;
    } else {
      printk("got stuck while homing floppy\n");
      break;
    }
  }
  _floppy_dis(handler);
}

static void floppy_seek(struct kswfloppy_data* handler,
                        unsigned int track) {
  if ((track > (FLOPPY_TRACKS -1)) || !(handler->flags & FLOPPY_FLAG_HOMED)) {
    return;
  }

  _floppy_en(handler);

  while (handler->cur_track != track) {
    if (handler->cur_track > track) {
      floppy_step(handler, FLOPPY_STEP_OUTWARDS);
    } else {
      floppy_step(handler, FLOPPY_STEP_INWARDS);
    }
  }

  _floppy_dis(handler);
}

#define STATE_WAIT_INDEX 0
#define STATE_READ 1
#define STATE_DONE 2

static int floppy_dump_track(struct kswfloppy_data* handler,
                              unsigned int track,
                              unsigned char* buffer) {
  unsigned int index = 0, data = 0, buf_ptr = 0;
  unsigned char state = STATE_WAIT_INDEX, data_byte = 0, bit_counter = 0, wait = 0;
  if (track > FLOPPY_TRACKS - 1) {
    return -EINVAL;
  }

  // seek to track
  floppy_seek(handler, track);

  // spin motor
  _floppy_en(handler);
  _spin_on(handler);

  while (state != STATE_DONE) {
    // first we wait until the index signal is asserted
    switch (state) {
    case STATE_WAIT_INDEX:
      read_signal(handler, _SIG_INDEX, &index);
      if (index) {
        state = STATE_READ;
        wait = 1;
      }
      break;
    case STATE_READ:
      read_signal(handler, _SIG_INDEX, &index);
      read_signal(handler, _SIG_RDATA, &data);
      if (wait && !index) {
        wait = 0;
      }
      if (index && !wait) {
        state = STATE_DONE;
        break;
      }

      data_byte |= (data<<(bit_counter++));
      if (bit_counter == 8) {
        bit_counter = 0;
        data_byte = 0;
        buffer[buf_ptr++] = data_byte;
        buf_ptr++;
      }

      udelay(1);
      break;
    default:
      break;
    }

  }

  printk("%d bytes read\n", buf_ptr);

  // stop motor
  _spin_off(handler);
  _floppy_dis(handler);

  return 0;
}


// probe, or add an instance
static int kswfloppy_probe(struct platform_device* pdev)
{
  struct kswfloppy_data* instance;
  unsigned int ii = 0;
  phandle pcap_phandle, mux_phandle;
  const struct property* prop;
  struct device_node* pcap_node, *mux_node;
  unsigned long long pin = 0;
  int err = 0;
  bool is_alt = 0;

  instance = (struct kswfloppy_data*)devm_kzalloc(&pdev->dev,
                                                  sizeof(struct kswfloppy_data),
                                                  GFP_KERNEL);

  if (!instance) {
    printk("probe failed\n");
    return -ENOMEM;
  }

  platform_set_drvdata(pdev, instance);

  prop = of_get_property(pdev->dev.of_node, "pcap-phandle", NULL);
  if (IS_ERR(prop)) {
    return PTR_ERR(prop);
  }
  pcap_phandle = be32_to_cpu(*(phandle*)prop);
  pcap_node = of_find_node_by_phandle(pcap_phandle);
  if (!pcap_node) {
    printk("cannot find timer handle\n");
    return -ENODEV;
  }
  if (!pcap_node->data) {
    printk("cannot get pulsecap data\n");
    return -ENODEV;
  }
  instance->pcap_inst = pcap_node->data;
  of_node_put(pcap_node);

  prop = of_get_property(pdev->dev.of_node, "mux,phandle", NULL);
  if (IS_ERR(prop)) {
    return PTR_ERR(prop);
  }
  mux_phandle = be32_to_cpu(*(phandle*)prop);
  mux_node = of_find_node_by_phandle(mux_phandle);
  if (!mux_node) {
    printk("cannot find mux handle\n");
    return -ENODEV;
  }
  if (!mux_node->data) {
    printk("cannot get mux data\n");
    return -ENODEV;
  }
  instance->mux_inst = mux_node->data;
  of_node_put(mux_node);

  err = of_property_read_u64(pdev->dev.of_node, "mux,pin", &pin);
  if (err) {
    printk("cannot get mux pin for pcap\n");
    return -EIO;
  }
  instance->capture_pin = pin;
  is_alt = of_property_read_bool(pdev->dev.of_node, "mux,alt");
  // configure mux
  aximux_set_src(instance->mux_inst, pin, is_alt ? 1 : 0);

  // allocate GPIOs
  for (ii=0; ii<15; ii++) {
    instance->floppy_gpios[ii] = devm_gpiod_get_index(&pdev->dev, "floppy", ii,
                                                      FLOPPY_SIGNAL_DIRECTIONS[ii]);
    if (IS_ERR(instance->floppy_gpios[ii])) {
      printk("failed to allocate gpio index %d with err %d\n", ii,
             (int)instance->floppy_gpios[ii]);
      return -ENODEV;
    }
  }

  deassert_all(instance);

  //assert_signal(instance, _SIG_MOTEB);
  floppy_home(instance);

  // test stepping
  //floppy_seek(instance, FLOPPY_TRACKS-1);
  floppy_dump_track(instance, 40, instance->dump);

  floppy_home(instance);

  printk("probed kswfloppy\n");
  return 0;
}

// remove an instance
static int kswfloppy_remove(struct platform_device* pdev)
{
  struct kswfloppy_data* instance = platform_get_drvdata(pdev);
  aximux_set_src(instance->mux_inst, instance->capture_pin, 0);
  deassert_all(instance);
  printk("kswfloppy removed\n");
  return 0;
}

// matching table
static struct of_device_id kswfloppy_of_ids[] = {
  { .compatible = "swfloppy-hack" },
};

// platform driver definition
static struct platform_driver kswfloppy_driver = {
  .probe = kswfloppy_probe,
  .remove = kswfloppy_remove,
  .driver = {
    .name = DRIVER_NAME,
    .of_match_table = of_match_ptr(kswfloppy_of_ids),
  },
};

// module initialization
static int kswfloppy_init(void)
{

  //register platform driver
  platform_driver_register(&kswfloppy_driver);

  return 0;
}

// module removal
static void kswfloppy_exit(void)
{
  //cleanup
  platform_driver_unregister(&kswfloppy_driver);
}

module_init(kswfloppy_init);
module_exit(kswfloppy_exit);


MODULE_DESCRIPTION("SW-direct floppy driver controller");
MODULE_LICENSE("GPL");
