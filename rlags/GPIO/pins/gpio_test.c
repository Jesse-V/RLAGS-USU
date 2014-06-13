#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h> 

#define GPIO_VAL 100
#define GPIO_LABEL "gpiotest"

static int __init gpiotest_init(void)
{
    if (!gpio_is_valid(GPIO_VAL)) {
        printk(KERN_CRIT "gpio_is_valid(%d) is false.\n", GPIO_VAL);
        return -EINVAL;
    }

    if (gpio_request(GPIO_VAL, GPIO_LABEL)) {
        printk(KERN_CRIT "gpio_request(%d, %s) failed.\n", 
                         GPIO_VAL, GPIO_LABEL);
        return -EINVAL;
    }
    
    gpio_free(GPIO_VAL);
    return 0;
}
module_init(gpiotest_init);

MODULE_LICENSE("GPL");
