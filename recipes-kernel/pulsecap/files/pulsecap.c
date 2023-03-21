// use AXI Timer to capture and measure pulses

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <asm/uaccess.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <asm/io.h>
#include <linux/spinlock.h>

#define DRIVER_NAME "esl-pulsecap"

//timer registers
#define REG_TCSR0 0x00
#define REG_TLR0 0x04
#define REG_TCR0 0x08
#define REG_TCSR1 0x10
#define REG_TLR1 0x14
#define REG_TCR1 0x18

// flags
#define TCSR0_ENALL (1<<10)
#define TCSR0_ENT0 (1<<7)
#define TCSR0_T0INT (1<<8)
#define TCSR0_ENIT0 (1<<6)
#define TCSR0_CAPT0 (1<<3)
#define TCSR0_MDT0 (1<<0)

// edge type
#define PULSECAP_EDGE_HIGH 0x00
#define PULSECAP_EDGE_LOW 0x01

// flags
#define PULSECAP_ENABLE 0x01
#define PULSECAP_DONE 0x02
#define PULSECAP_EDGES_READ 0x04
#define PULSECAP_CANCELLED 0x08

#define MAX_PULSECAP_INSTANCES 4

// states
#define PULSECAP_STATE_IDLE 0
#define PULSECAP_STATE_RUN 1
#define PULSECAP_STATE_DONE 2

typedef unsigned int edge_count_t;

struct pulse_data {
  edge_count_t edge_count;
  unsigned char edge_type;
};

// max amount of edges that can be captured
#define PULSECAP_MAX_COUNT PAGE_SIZE/sizeof(edge_count_t)

struct pulsecap_data {
  int irqnum;
  void __iomem *regs;

  struct pulse_data stored_edges[PULSECAP_MAX_COUNT];
  unsigned int edge_ptr;
  unsigned int edge_target;
  unsigned char flags;
  spinlock_t lock;
  unsigned int addr;
};

// TODO: use linked lists for instances, not an array, prone to failures
struct pulsecap_module_data {
  unsigned int instance_count;
  struct pulsecap_data* instances[MAX_PULSECAP_INSTANCES];
  u32 dev_major;
  dev_t devt;
};

static struct pulsecap_module_data module_data;

static inline u32 reg_read(struct pulsecap_data* dev, u32 reg)
{
  return ioread32(dev->regs + reg);
}

static inline void reg_write(struct pulsecap_data* dev, u32 reg, u32 value)
{
  iowrite32(value, dev->regs + reg);
}

static void stop_timer(struct pulsecap_data* dev)
{
  u32 regval;
  regval = reg_read(dev, REG_TCSR0);
  regval &= ~TCSR0_ENT0;
  reg_write(dev, REG_TCSR0, regval);

  regval = reg_read(dev, REG_TCSR1);
  regval &= ~TCSR0_ENT0;
  reg_write(dev, REG_TCSR1, regval);
}

static void disable_int_0(struct pulsecap_data* dev)
{
  u32 regval;
  printk("int dis 0\n");
  regval = reg_read(dev, REG_TCSR0);
  regval &= ~TCSR0_ENIT0;
  reg_write(dev, REG_TCSR0, regval);
}

static void disable_int_1(struct pulsecap_data* dev)
{
  u32 regval;
  printk("int dis 1\n");
  regval = reg_read(dev, REG_TCSR1);
  regval &= ~TCSR0_ENIT0;
  reg_write(dev, REG_TCSR1, regval);
}

static void disable_interrupts(struct pulsecap_data* dev)
{
  disable_int_0(dev);
  disable_int_1(dev);
}

/*static void enable_int_0(struct pulsecap_data* dev)
{
  u32 regval;
  printk("int en 0\n");
  regval = reg_read(dev, REG_TCSR0);
  regval |= TCSR0_ENIT0;
  reg_write(dev, REG_TCSR0, regval);
}

static void enable_int_1(struct pulsecap_data* dev)
{
  u32 regval;
  printk("int en 1\n");
  regval = reg_read(dev, REG_TCSR1);
  regval |= TCSR0_ENIT0;
  reg_write(dev, REG_TCSR1, regval);
}

static void enable_interrupts(struct pulsecap_data* dev)
{
  enable_int_0(dev);
  enable_int_1(dev);
  }*/

static irqreturn_t timer_irq_handler(int irq, void* data)
{
  struct pulsecap_data* drv = data;
  u32 t0_state, t1_state;
  unsigned char edge_type;
  unsigned int edge_count;

  // read registers
  spin_lock(&drv->lock);
  t0_state = reg_read(drv, REG_TCSR0);
  t1_state = reg_read(drv, REG_TCSR1);

  if  ((drv->flags & PULSECAP_ENABLE) && !(drv->flags & PULSECAP_DONE))
    {
      // figure out which edge we are in
      if (t0_state & TCSR0_T0INT)
        {
          // level HIGH
          edge_type = PULSECAP_EDGE_HIGH;
          //disable_int_0(drv);
          edge_count = ioread32(drv->regs + REG_TLR0);
          //enable_int_1(drv);
        }
      else
        {
          edge_type = PULSECAP_EDGE_LOW;
          //disable_int_1(drv);
          edge_count = ioread32(drv->regs + REG_TLR1);
          //enable_int_0(drv);
        }

      if (drv->edge_ptr > 0)
        {
          // check last edge
          if (drv->stored_edges[drv->edge_ptr-1].edge_type != edge_type)
            {
              // store
              drv->stored_edges[drv->edge_ptr].edge_type = edge_type;
              drv->stored_edges[drv->edge_ptr].edge_count = edge_count;

              // increment pointer
              if (drv->edge_ptr < drv->edge_target)
                {
                  drv->edge_ptr++;
                }
              else
                {
                  // done
                  stop_timer(drv);
                  drv->flags |= PULSECAP_DONE;
                }
            }
        }
      else
        {
          if (edge_type == PULSECAP_EDGE_HIGH)
            {
              // first edge is always HIGH
              drv->stored_edges[0].edge_type = edge_type;
              drv->stored_edges[0].edge_count = edge_count;
              drv->edge_ptr = 1;
            }
        }
    }
  // clear interrupt flags
  t0_state = ioread32(drv->regs+REG_TCSR0);
  t1_state = ioread32(drv->regs+REG_TCSR1);
  t0_state |= TCSR0_T0INT;
  iowrite32(t0_state, drv->regs + REG_TCSR0);
  t1_state |= TCSR0_T0INT;
  iowrite32(t1_state, drv->regs + REG_TCSR1);
  spin_unlock(&drv->lock);

  return IRQ_HANDLED;
}

static void initialize_timer(struct pulsecap_data* dev)
{
  // enable interrupts and setup capture mode
  reg_write(dev, REG_TCSR0, TCSR0_T0INT|TCSR0_ENIT0|TCSR0_CAPT0|TCSR0_MDT0);
  reg_write(dev, REG_TCSR1, TCSR0_T0INT|TCSR0_ENIT0|TCSR0_CAPT0|TCSR0_MDT0);

  //reset everything else
  reg_write(dev, REG_TLR0, 0);
  reg_write(dev, REG_TLR1, 0);
}

static void start_timer(struct pulsecap_data* dev)
{
  u32 regval;
  regval = reg_read(dev, REG_TCSR0);
  regval |= TCSR0_ENALL;
  reg_write(dev, REG_TCSR0, regval);
}

int pulsecap_start_capture(struct pulsecap_data* pcap, unsigned int edge_count) {
  if (!pcap) {
    return -ENOENT;
  }
  if (pcap->flags & PULSECAP_ENABLE) {
    if (!(pcap->flags & PULSECAP_DONE)) {
      return -EBUSY;
    }
  }

  if (edge_count > PULSECAP_MAX_COUNT) {
    return -EINVAL;
  }

  // setup timers
  initialize_timer(pcap);

  // store count
  pcap->edge_target = edge_count;
  pcap->edge_ptr = 0;

  // flag enabled
  pcap->flags |= PULSECAP_ENABLE;
  pcap->flags &= ~PULSECAP_DONE;
  pcap->flags &= ~PULSECAP_EDGES_READ;
  pcap->flags &= ~PULSECAP_CANCELLED;

  // start timers
  start_timer(pcap);

  return 0;
}
EXPORT_SYMBOL(pulsecap_start_capture);

unsigned int pulsecap_edges_captured(struct pulsecap_data* pcap) {
  if (!pcap) {
    return 0;
  }
  if (pcap->flags & (PULSECAP_ENABLE | PULSECAP_CANCELLED)) {
    return pcap->edge_ptr;
  } else if (pcap->flags & PULSECAP_DONE) {
    return pcap->edge_target;
  }
}
EXPORT_SYMBOL(pulsecap_edges_captured);

int pulsecap_cancel_capture(struct pulsecap_data* pcap) {
  if (!pcap) {
    return -ENOENT;
  }
  if (!pcap->flags & PULSECAP_ENABLE) {
    return -EIO;
  }

  // stop
  disable_interrupts(pcap);
  stop_timer(pcap);
  pcap->flags &= ~PULSECAP_ENABLE;
  pcap->flags &= ~PULSECAP_DONE;
  pcap->flags |= PULSECAP_CANCELLED;

  return 0;
}
EXPORT_SYMBOL(pulsecap_cancel_capture);

int pulsecap_get_state(struct pulsecap_data* pcap) {
  if (!pcap) {
    return -ENOENT;
  }

  if (pcap->flags & PULSECAP_ENABLE) {
    if (pcap->flags & PULSECAP_DONE) {
      return PULSECAP_STATE_DONE;
    }
    else {
      return PULSECAP_STATE_RUN;
    }
  }
  return PULSECAP_STATE_IDLE;
}
EXPORT_SYMBOL(pulsecap_get_state);

int pulsecap_read_edges(struct pulsecap_data* pcap,
                        unsigned int count,
                        unsigned int* dest) {
  unsigned int ii = 0;
  if (!pcap || !dest) {
    return -ENOENT;
  }
  if (pcap->flags & PULSECAP_DONE) {
    if (count > pcap->edge_target) {
      count = pcap->edge_target;
    }
  } else if (pcap->flags & PULSECAP_CANCELLED) {
    if (count > pcap->edge_ptr) {
      count = pcap->edge_ptr;
    }
  } else {
    return -EBUSY;
  }

  for (ii=0; ii<count; ii++) {
    *(dest++) = pcap->stored_edges[ii].edge_count;
  }

  return count;
}
EXPORT_SYMBOL(pulsecap_read_edges);

int pulsecap_get_max_edges(struct pulsecap_data* pcap) {
  if (!pcap) {
    return -ENODEV;
  }
  return PULSECAP_MAX_COUNT;
}
EXPORT_SYMBOL(pulsecap_get_max_edges);

static ssize_t capture_store(struct device* dev, struct device_attribute* attr,
                             const char* buf, size_t count)
{
  struct pulsecap_data* drv = dev_get_drvdata(dev);
  unsigned long value = 0;
  int err;
  //get value
  err = kstrtoul(buf, 10, &value);
  if (err < 0) {
    //failed
    return err;
  }
  err = pulsecap_start_capture(drv, value);
  if (err) {
    return err;
  }
  return count;
}

static ssize_t capture_show(struct device* dev, struct device_attribute* attr, char* buf)
{
  struct pulsecap_data* drv = dev_get_drvdata(dev);
  unsigned int edges = 0;
  edges = pulsecap_edges_captured(drv);
  return sprintf(buf, "%u\n", edges);
}

static ssize_t cancel_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
  struct pulsecap_data* drv = dev_get_drvdata(dev);
  int err = 0;
  err = pulsecap_cancel_capture(drv);
  if (err) {
    return err;
  }
  return count;
}

static ssize_t state_show(struct device* dev, struct device_attribute* attr, char* buf)
{
  struct pulsecap_data* drv = dev_get_drvdata(dev);
  int state = pulsecap_get_state(drv);

  if (state < 0) {
    return state;
  }

  switch (state) {
  case PULSECAP_STATE_DONE:
    return sprintf(buf, "done\n");
  case PULSECAP_STATE_RUN:
    return sprintf(buf, "running\n");
  case PULSECAP_STATE_IDLE:
    return sprintf(buf, "idle\n");
  default:
    return -EIO;
  }
}

static ssize_t edge_data_show(struct device* dev, struct device_attribute* attr, char* buf)
{
  struct pulsecap_data* drv = dev_get_drvdata(dev);
  unsigned int* bufPtr = (unsigned int*)buf;
  int count = 0;

  printk("pulsecap: reading out %u edges\n", drv->edge_target);
  count = pulsecap_read_edges(drv, drv->edge_target, bufPtr);
  if (count < 0) {
    return count;
  }
  drv->flags |= PULSECAP_EDGES_READ;
  return count*sizeof(unsigned int);
}

static ssize_t max_edges_show(struct device* dev, struct device_attribute* attr, char* buf)
{
  struct pulsecap_data* drv = dev_get_drvdata(dev);
  int count = pulsecap_get_max_edges(drv);
  if (count < 0){
    return count;
  }
  return sprintf(buf, "%lu\n", (unsigned long)count);
}

static struct device_attribute capture_attr = {
  .attr = {
    .name = "capture",
    .mode = S_IWUSR | S_IRUGO,
  },
  .show = capture_show,
  .store = capture_store,
};

static struct device_attribute cancel_attr = {
  .attr = {
    .name = "cancel",
    .mode = S_IWUSR,
  },
  .store = cancel_store,
};

static struct device_attribute state_attr = {
  .attr = {
    .name = "state",
    .mode = S_IRUGO,
  },
  .show = state_show,
};

static struct device_attribute edge_data_attr = {
  .attr = {
    .name = "edge_data",
    .mode = S_IRUGO,
  },
  .show = edge_data_show,
};

static struct device_attribute max_edges_attr = {
  .attr = {
    .name = "max_edges",
    .mode = S_IRUGO,
  },
  .show = max_edges_show,
};

static struct attribute *pulsecap_attrs[] = {
  &capture_attr.attr,
  &cancel_attr.attr,
  &state_attr.attr,
  &edge_data_attr.attr,
  &max_edges_attr.attr,
  NULL
};

static struct attribute_group pulsecap_attr_group = {
  .name = "control",
  .attrs = pulsecap_attrs,
};

static const struct attribute_group* pulsecap_attr_groups[] = {
  &pulsecap_attr_group,
  NULL
};

static struct of_device_id esl_pulsecap_of_ids[] = {
  { .compatible = "pulsecap-timer" },
  {}
};

static struct class pulsecap_class = {
  .name = "pulsecap",
  .owner = THIS_MODULE,
};

static int esl_pulsecap_remove(struct platform_device* pdev)
{
  struct pulsecap_data* inst = platform_get_drvdata(pdev);

  //disble interrupts
  disable_interrupts(inst);
  stop_timer(inst);

  //iounmap(inst->regs);

  //destroy device
  device_destroy(&pulsecap_class, MKDEV(module_data.dev_major, module_data.instance_count-1));

  module_data.instance_count--;
  return 0;
}


static int esl_pulsecap_probe(struct platform_device* pdev)
{
  int err;
  struct pulsecap_data* inst;
  struct resource* res;
  struct device* dev;
  dev_t devno = MKDEV(module_data.dev_major, module_data.instance_count);

  // allocate instance
  inst = devm_kzalloc(&pdev->dev, sizeof(struct pulsecap_data), GFP_KERNEL);
  if (!inst)
    {
      return -ENOMEM;
    }

  spin_lock_init(&inst->lock);

  printk(KERN_INFO "pulsecap: loading with device: %s\n", pdev->name);

  // map I/O memory
  res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  inst->regs = devm_ioremap_resource(&pdev->dev, res);
  if (IS_ERR(inst->regs))
    {
      return PTR_ERR(inst->regs);
    }

  // get interrupt (unreliable, wrong values)
  res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
  if (!res)
    {
      printk(KERN_ERR "pulsecap: could not get IRQ number\n");
      return -EIO;
    }

  printk(KERN_INFO "pulsecap: got IRQ %u\n", res->start);
  inst->irqnum = res->start;

  err = devm_request_irq(&pdev->dev, inst->irqnum,
                         timer_irq_handler,
                         IRQF_TRIGGER_RISING,
                         pdev->name,
                         inst);
  if (err < 0)
    {
      printk(KERN_ERR "pulsecap: could not request interrupt\n");
      return -EIO;
    }

  //set data
  platform_set_drvdata(pdev, inst);
  pdev->dev.of_node->data = inst;

  dev = device_create_with_groups(&pulsecap_class, &pdev->dev,
                                  devno, inst,
                                  pulsecap_attr_groups,
                                  "pulsecap%d", module_data.instance_count);

  if (IS_ERR(dev))
    {
      return PTR_ERR(dev);
    }

  module_data.instances[module_data.instance_count] = inst;
  module_data.instance_count++;

  printk("pulsecap: probed\n");

  return 0;
}


static struct platform_driver esl_pulsecap_driver = {
  .probe = esl_pulsecap_probe,
  .remove = esl_pulsecap_remove,
  .driver = {
    .name = DRIVER_NAME,
    .of_match_table = of_match_ptr(esl_pulsecap_of_ids),
  },
};

static int __init esl_pulsecap_init(void)
{

  int err;

  err = alloc_chrdev_region(&module_data.devt, 0, MAX_PULSECAP_INSTANCES, "pulsecap");
  if (err < 0)
    {
      return err;
    }
  module_data.instance_count = 0;
  module_data.dev_major = MAJOR(module_data.devt);

  class_register(&pulsecap_class);

  platform_driver_register(&esl_pulsecap_driver);

  return 0;
}

static void __exit esl_pulsecap_exit(void)
{
  platform_driver_unregister(&esl_pulsecap_driver);
  class_destroy(&pulsecap_class);

  unregister_chrdev_region(module_data.devt, MAX_PULSECAP_INSTANCES);
}

module_init(esl_pulsecap_init);
module_exit(esl_pulsecap_exit);

MODULE_DESCRIPTION("Pulse capture and measurement using AXI Timer");
MODULE_LICENSE("GPL");
