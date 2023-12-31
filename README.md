# nrf_io_practice

gpio practice repo for nrf in nrf connect sdk (NCS)
Feel free to use whatever here you need, there is some clean up to be done.

# Overview
A simple button demo showcasing the use of GPIO input with interrupts.
The sample prints a message to the console each time a button is pressed.
Functions pretty similarly to my caf sample. I don't really account for bouncy buttons here.

# Function
- Press button 1 twice quickly (within 1s) to alternate LED scroll speed.
- Press button 1 twice slowly, you'll get a print message telling you that you were slow on the second press.
- Press button 2 to switch which direction the scrolling is in.

**!! This video is from [droidecahedron/caf_click](https://github.com/droidecahedron/nrf_caf_click) !!**
> I just did not feel like re-recording and re-uploading. It's pretty much the same, try it yourself.
> 
https://user-images.githubusercontent.com/63935881/264470500-d66a88fd-9624-4068-be93-851c2d8ae153.mp4

> example of slow button press terminal output... speed wont change. You might see speed change if the switch bounces a lot. These switches on the nrf dks can wear out and be bouncy.

![image](https://github.com/droidecahedron/nrf_io_practice/assets/63935881/f0c1f42c-c220-4114-8f7b-a181ec1ef428)


# Requirements
## nRF52840DK
>  (but honestly it should work on the 52833, 7002, 5340dk as well as ublox nina-b3, laird bl653/4, and fanstel bc840 modules (as long as there arent device tree issues.))

<img src="https://github.com/droidecahedron/nrf-blueberry/assets/63935881/12612a0e-9f81-4431-8b22-f69704248f89" width=25% height=25%>

The board hardware must have a push button connected via a GPIO pin. These are
called "User buttons" on many of Zephyr's :ref:`boards`.


# Devicetree details
This section provides more details on devicetree configuration.

Here is a minimal devicetree fragment which supports this sample. This only
includes a ``sw0`` alias; the optional ``led0`` alias is left out for
simplicity.

```
   / {
   	aliases {
   		sw0 = &button0;
   	};

   	soc {
   		gpio0: gpio@0 {
   			status = "okay";
   			gpio-controller;
   			#gpio-cells = <2>;
   			/* ... */
   		};
   	};

   	buttons {
   		compatible = "gpio-keys";
   		button0: button_0 {
   			gpios = < &gpio0 PIN FLAGS >;
   			label = "User button";
   		};
   		/* ... other buttons ... */
   	};
   };
```
As shown, the ``sw0`` devicetree alias must point to a child node of a node
with a "gpio-keys" :ref:`compatible <dt-important-props>`.

The above situation is for the common case where:

- ``gpio0`` is an example node label referring to a GPIO controller
-  ``PIN`` should be a pin number, like ``8`` or ``0``
- ``FLAGS`` should be a logical OR of :ref:`GPIO configuration flags <gpio_api>`
  meant to apply to the button, such as ``(GPIO_PULL_UP | GPIO_ACTIVE_LOW)``

This assumes the common case, where ``#gpio-cells = <2>`` in the ``gpio0``
node, and that the GPIO controller's devicetree binding names those two cells
"pin" and "flags" like so:

```yaml
   gpio-cells:
     - pin
     - flags
```

This sample requires a ``pin`` cell in the ``gpios`` property. The ``flags``
cell is optional, however, and the sample still works if the GPIO cells
do not contain ``flags``.

# Building and Running
After startup, the program looks up a predefined GPIO device, and configures the
pin in input mode, enabling interrupt generation on falling edge. During each
iteration of the main loop, the state of GPIO line is monitored and printed to
the serial console. When the input button gets pressed, the interrupt handler
will print an information about this event along with its timestamp.

# Extra Documentation
https://docs.zephyrproject.org/latest/kernel/services/timing/timers.html
