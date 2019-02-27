# README #

This repository is the software and 3D part source for a "Connect-4" STEM project.

### Parts to purchase ###

* Purchase [wooden Connect Four game](https://www.target.com/p/connect-4-rustic-series-board-game/-/A-52118927?ref=&ref=OpsEmail_Order_635&j=46563&sfmc_sub=278578531&l=20_HTML&u=23341750&mid=7284873&jb=337050) for $19
* Purchase [aluminum slider](https://www.sparkfun.com/products/12500) for $12
* Purchase [channel slider](https://www.sparkfun.com/products/12560) for $5
* Purchase 2 [ball bearings](https://www.sparkfun.com/products/13277) for $2
* Purchase [stepper motor](https://www.amazon.com/gp/product/B00EYIFW70/ref=ppx_yo_dt_b_asin_title_o07__o00_s00?ie=UTF8&psc=1) for $17
* Purchase [Induino Mega](https://www.amazon.com/gp/product/B01G6Z1EYI/ref=ppx_yo_dt_b_asin_title_o04__o00_s00?ie=UTF8&psc=1) for $14 (or any Arduino Mega compatible board)
* Purchase [Adafruit motor shield](https://www.amazon.com/gp/product/B00PUTH3B0/ref=oh_aui_search_asin_title?ie=UTF8&psc=1) for $19
* Purchase [Timing belt, pulley, and tensioner](https://www.amazon.com/gp/product/B07FYYGFGQ/ref=ppx_yo_dt_b_asin_title_o09__o00_s00?ie=UTF8&psc=1) for $13
* Purchase [Stacking headers](https://www.amazon.com/gp/product/B00LB76EVU/ref=ppx_yo_dt_b_asin_title_o00__o00_s00?ie=UTF8&psc=1) for $6
* Purchase [Ribbon cable](https://www.amazon.com/gp/product/B01D9KISHY/ref=ppx_yo_dt_b_asin_title_o09__o00_s01?ie=UTF8&psc=1) for $8
* Purchase [LCD screen](https://www.amazon.com/gp/product/B01E6N19YC/ref=ppx_yo_dt_b_asin_title_o07__o00_s00?ie=UTF8&psc=1) for $10
* Purchase [Servo](https://www.amazon.com/gp/product/B07C5PGD3Q/ref=oh_aui_search_asin_title?ie=UTF8&psc=1) for $11
* Purchase 7 [photo interrupters](https://www.digikey.com/product-detail/en/sharp-socle-technology/GP1A57HRJ00F/1855-1043-ND/718287) for $28
* Purchase [red](https://www.adafruit.com/product/1445) and [green](https://www.adafruit.com/product/1504) momentary pushbuttons for $2
* Purchase 7 330ohm 1/4w resistors
* Purchase a micro-switch
* Purchase a [battery and charger](https://www.overstock.com/Emergency-Preparedness/Large-Capacity-9800MAH-DC12V-Portable-Rechargeable-Li-ion-Battey-Pack/17432557/product.html?cid=273569&ehid=B6C9DCA0B72C1C57E040010A239C3C94&fp=f&token=273569-2735692019012215481894286427331070117747080812-1-f8cae0&utm_campaign=t_20160908_shipconf&utm_medium=email&utm_source=strongview) for $30

* You will need a 3D printer and filament. We used ABS, though presumably PLA also works.
* You will also want wood screws of various sizes and metal bolt/nut combos, mostly of 3mm size.
* For nicer connections, you'll also want to consider soldering iron/solder, heat shrink, and a Dupont connector kit.

### Construction of robot ###

The following parts are all available here print. They can be printed on
an inexpensive hobbyist 3D printer with a 150mm x 150mm x 150mm printing
volume. We used ABS filament for durability and easy in welding pieces
together with acetone. No acrylic parts are needed.

These are the parts for the gantry dropper mechanism above the board:

* [gantry hopper holder](parts/PatrickMcCabe/Piece Holder.STL)
* [2x belt clamp](parts/PatrickMcCabe/Belt Clamp.STL)
* [gantry slider top](parts/PatrickMcCabe/Slider Top.STL)
* [2x bearing block](parts/PatrickMcCabe/Bearing Block.STL)
* [Motor mount](parts/PatrickMcCabe/Motor Mount.STL)
* [Combined idler and post](parts/combined-idler-post.stl)
* [Disc Funnel](parts/PatrickMcCabe/Slider Spacer Front.STL)
* [Servo Releaser](parts/thinner-servo-releaser.stl)
* [Disc hopper](parts/taller-gantry-hopper.stl)

To build the gantry, which is the slider that slides across the top of
the board and drops discs into the board:

* Connect the stepper motor to the Motor Mount with screws.
* Connect the motor mount to the 15" C channel in the second to left-most
  screw holes.
* Insert the bearings into the two bearing blocks. This is a very tight
  fit. I used a pair of pliers to squeeze the bearing into the block to be
  flush. This damaged one of my bearings and I needed to purchase a new one.
  Be careful to only squeeze the outermost ring of the bearing to avoid bending
  the internal mechanism and causing friction when the bearing moves.
* Insert the combined idler and rod into the two bearings.
* Screw the two bearing blocks into the C channel, aligned with the rightmost
  screw holes. The bearing blocks did not fit well into the channel and would
  cause the channel to bend outward and increase the friction of the slider
  against the channel. I ended up significantly sanding down the two bearing
  blocks so they fit into the channel without bending it. Sorry I did not get
  around to redesigning the parts to be smaller. Likely they are about 1mm too
  thick.
* Place the C clamps on the C-channel.
* Screw the top of the C clamps into the gantry slider top.
* Screw the front of the C clamps into the disc funnel.
* Screw the gantry hopper holder into the left side of the C clamps.
* Screw the servo onto the top of the gantry slider top.
* Cut the servo arm to be less than 4mm and screw the Servo Releaser
  into the servo arm.
* Superglue the servo arm onto the servo drive to avoid accidental disloding.
* Screw the gantry hopper into the gantry hopper holder.
* Attach the belt pulley to the stepper motor with set screw included with
  pulley.
* Wrap the belt drive around the idler and the stepper pulley, pull moderately tight.
* Cut belt to 15cm longer than distance to gantry slider top.
* Route the top sides of the belt loop up into the gantry slider top, pulled around
  to the outside of the gantry slider top.
* Clamp down the belt drive with 2 Belt Clamps and 4 screws.
* Tighten the belts with spring tensioners.
* Connect the microswitch to the C channel. See picture for positioning.
* Consider soldering a 3-pin connector like that shown to the microcontroller
  to make it easier to take the device apart in the future.

At this point the next best step is to attach the 7 photo-interrupters to
the top of the board. This is probably the most labor intensive step. Each
photo-interrupter has an emitter side (2 pins) and a phototransistor side (3
pins). On both sides longest lead is ground. The middle lead of the phototransistor
is the output. The other lead is VCC. Since the emitter is just a simple
IR LED, current limiting is required to avoid burning out the LED. Each
LED power needs a 330V 1/4W resistor between it and actual 5V input. The
phototransistor is actually a complicated circuit and does not need current
limiting.

The photo-interrupters come with emitter and receiver sides connected by a
plastic spacer. This spacer is similar to the width of the Connect 4 board,
so it is tempting to think they can just be used as is. However, the spacer
is too thick and discs will not fit through the spacers (they would leave
less than 1 1/4" wide, the diameter of the discs). As such, the photo-interruptors
call be cut easily with wire cutters to give full access to the drop area
at the top of the board.

But before cutting the photo-interrupters they should be wired up and attached
to the board. In my case I put the phototransistor side at the back of the board
(since we need 7 signal wires coming out from the back of the board, which is
unsightly), and emitter side at the front. For the emitter and phototransistors,
the same GND and 5V wires were used. These wires plus the 7 signal wires come
from the same ribbon cable. For convenience I used white wire for 5V, black for GND,
and purple through brown for the 7 columns, using 9 of the 10 wires of the 10-wire
ribbon cable. Additionally, the 5V and GND side of the microswitch and its output
should be connected to this cable. The output signal should be wired to gray.

A bus wire was used to solder all GND connections together. On the emitter side
since all emitters require 330ohm biasing resistors I used heat-shrink to hide
the resistors and insulate the 5V wire which was chained between each emitter
positive side.

To reduce surplus wiring in this visible part of the board, I used a paper template
of the top of the board, with exact markings of each column and spaced, soldered,
and heat-shrunk the wires for the photo-interruptors. The spacer of the
photo-transistors was still intact at this stage. Finally when all wires were
intact and connected to the 10-wire ribbon cable, the spacers were cut for all 7
interruptors. The photo-interrupters were then glued to the top of the board
with a hotglue gun. When glued, make sure that the emitter and receiver sides of
the photo-interrupters are as flush as possible with the opening of the board.
If they are too wide, discs can get stuck on the top of the board when they pass
them. If they are too narrow, discs can get stuck on top of the photo interrupter.
Any excess glue towards the inside of the board can be clipped away for a smooth
transition from the funnel, past photo-interrupter, into the board.

Next the feet and slide should be printed and attached to the board.

Print these parts:

* [back-right foot](parts/foot.stl)
* [back-left battery-holder foot](parts/battery-foot.stl)
* [front-left controller foot](parts/controller-foot.stl)
* [front-right hopper-foot](parts/hopper-foot.stl)
* [controller top](parts/controller-top.stl)
* [Slide left](parts/slide-left.stl)
* [Slide right](parts/slide-right.stl)

Use acetone to weld the slide left and slide right pieces together into one
large 25cm wide piece.

Each foot connects at the bottom of the foot and optionally on the sides
with a screw. Drill pilot holes into the connect4 board. Try to make the
feet of the two sides as close to flush and level with each other as possible.

Connect the battery mount to the back left, controller mount to front left,
disc holder foot to front right, and regular foot to back right. Once these
are well secured by screws, attach the slide on top of the feet. Screw the slide
into place. The cutout at the back of the slide may be too small for the connect4
board latch that closes the bottom of the board so the discs don't fall out.
If so, use a file or sandpaper to make it wide enough to give proper clearance.

At this point you will want to wire up the controller box. Place the I2C LCD
panel onto the spacer rods inside the controller box. Use another 10-lead ribbon
cable, using black for GND and white for 5V. Use gray and purple leads for
I2C SDA and SDL signals. Use green for yes button signal and red for no button
signal. The other wires can go unused. Insert the momentary pushbuttons into
the controller top piece. For future maintenance you may want to solder each
of these buttons to connectors when then connect to the ribbon cable wires.
The "signal" for the momentary push-buttons is actually a ground pull-down.
This means the push-buttons should be wired to GND on one side and the other
is the "signal" back to the GPIO pins. The GPIO pins will be configured in
pull-up mode so that they read 5V when button is not pressed and 0V/GND when
pressed.

Leave roughly 15cm for this ribbon cable. Put a dupont female 4-pin end on
the black through purple wires. Put a 2 pin end up the red and green wires.

Screw the controller top into the controller foot.

Finally, insert the battery into the battery holder, with the power plugs
oriented to the front bottom and the LED and switch oriented to the back
bottom of the battery holder. Pull the battery connectors out so they are coming
towards the controller foot.

At this point the base is stable enough to put the gantry on the board.

These are the parts for connecting the gantry to the board:

* [Left gantry connector](parts/gantry-connector-left.stl)
* [Right gantry connector](parts/gantry-connector-right.stl)

To attach the gantry to the board:

* Align the connectors to the very top of the wooden legs of the Connect-4 board.
* Level both the left and right connectors.
* Carefully align the gantry with the top of the board. Several things to keep
  in mind. There needs to be enough vertical clearance for the C sliders to
  not touch the photo-interruptor plastic or any of the wires connecting to it.
  Try placing discs through the funnel and make sure they clear the edges of the
  photo-interruptors and the board.
* Once aligned, carefully drill a 1/8" hole through the screw-holes in the bottom
  of the C channel and the left and right gantry connectors. To reduce potential
  for the gantry to slip forward and back, squeeze the connectors in towards the
  center of the board slightly when drilling the hole. This will assure the screws
  are slightly pulling the C channel away from the center and this spring will keep
  the gantry from moving forward and back.

Now that the gantry and feet are connected, the final step is wiring up the
Arduino, motor boards, and mounting them in their enclosure.

These are the parts for the enclosure

* [Arduino enclosure bottom](parts/arduino-enclosure-bottom.stl)
* [Arduino enclosure top](parts/arduino-enclosure-top.stl)

Screw the enclosure bottom into the left leg of the connect4 board, positioning
it slightly (~ 1/4 inch) below the left gantry connector. There should be a
ribbon cable passage on the bottom of the enclosure and on the back side
of the enclosure, if oriented properly.

If you received the same adafruit motor board, the motor board needs to have
stacking connectors soldered to it. Solder these connectors.

We are going to need to modify the motor board for three more reasons:

* The display also needs to share the I2C bus and VCC and GND with the motor board
* The controller buttons need to be connected to GPIOs.
* The gantry also needs to share the VCC and GND with the motor board.

To achieve all of these, we get VCC and GND by adding female connectors to the
motor board. We use the blank breadboard in the middle of the motor board to
break out access to D8 (no button) and D9 (yes button).

Screw the Arduino Mega board into the Arduino enclosure bottom.

Route the gantry ribbon cable into the enclosure in the back ribbon cable passage.
Route the controller ribbon cable into the enclosure via the bottom ribbon cable
passage.

Attach gray wire (micro-switch) from gantry ribbon cable to A8, attach the rest
of the gantry wires to A9-A15. The Dupont header you created earlier makes this
an easy connection directly to the Arduino Mega board.

You can now connect the servo to the motor board. Connect the servo 3-pin connector
to the servo extension cable (white extension wire should connect to orange servo
wire). Then route the extension cable through the back ribbon cable passage. Plug
the extension cable into the motor board servo 2 3-pin male connector.

For the controller ribbon cable, attach the I2C cables (gray and purple) to the
female stacking connectors you soldered onto the motor board. Attach the
green and red signal wires from the push button to the D8 and D9 connectors you
soldered on earlier.

Stack the motor board on top of the arduino board.

Finally, take the Arduino enclosure top and slide it onto the Arduino enclosure
bottom.

### Building code ###

At this point you are ready to start building and running code.

Clone this repository into $HOME/prj/connect4-stem

Install the arduino SDK. For simplicity install it to $HOME/prj/arduino-1.8.8.
If the version number is different or you decide to install it to a different
directory, you will just need to either modify the Makefile or pass a different
ADIR variable into the make command.

You will need to have installed host compiler tools such as g++ and terminal
emulator picoterm.

```
$ sudo apt-get install g++ picoterm
```

Now begin to build:

```
$ cd $HOME/prj/connect4-stem/src
$ make
```

Note that this makefile uses the Arduino SDK command-line tools directly.
Using .ino and .dce files is convenient for beginners using Arduino GUI.
However to support unit testing and building the same code on host, a
more powerful build environment is needed. This makefile builds binary,
converts it to flashable version and then ultimately flashes it to device.

Just running make will cause host versions and arduino versions of code to be
built. The host versions allow us to unit test various code before it is
put on device. The unit tests will automatically run and device code will
only be built if the unit tests succeed (which they should in any clean
checkout).

To first run code on Arduino, do the follow:

* Make sure your host computer is connected to Arduino by USB
* Make sure battery is charged and plugged into Arduino and turned on.

```
$ make run
```

The code will be built and flashed to the Arduino. The make command will now
run picoterm to show serial output from the code.

IMPORTANT: Never flash another program to Arduino unless you really know
what you are doing! This is because you have now connected various digital
GPIOs and analog GPIOs to be inputs and outputs. If you flash different
code that changes input pins to output pins, you run the risk of shorting
the Arduino Mega board, and may need to replace it.

### Testing ###

Besides the unit test, the Arduino code has diagnostic mode. When showing
"Do you want to play?" you can push both yes and no buttons simultaneously
and a diagnostic mode will be entered. This allows you to run a dropper test.
This test will find the home position (step backwards until "home" microswitch
is pressed by the slider), advance forward to each column in succession,
use the servo to drop the disc into the column, wait for the disc to pass
the photo-interrupter. After a disc is placed into each of the 7 columns
successfully the test is complete. When this test completes you can be
moderately assured that the game mode will work.
