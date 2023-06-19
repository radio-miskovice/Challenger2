# Challenger Keyer

## The Story

I have started writing Challenger Keyer firmware after nightmare experience with configuring K3NG code for several
versions of hardware ([OK1RR](https://ok1rr.com) "smallest keyer" [Tinykeyer](https://ok1rr.com/technical-topics/the-tinykeyer/), OK1RR BigKeyer variant of K5BCQ, my own-designed hardware).

### Avoiding the K3NG configuration nightmare (partially)

In the meantime, I also built [Spider Keyer](http://ok1fig.nagano.cz/SpiderKeyer/SpiderKeyer.htm)
by [Peter, OK1FIG](https://ok1fig.nagano.cz), and burned his firmware in. What I liked
most about Peter's code is his successful endeavour to unmessify and unspaghettify the
original K3NG code by reducing features and options the the absolute minimum needed.
What I liked at first was also his decision to design his [own communication protocol](http://ok1fig.nagano.cz/SpiderKeyer/SpiderKeyerSpecs.htm)
("Spider protocol"), a fresh, from-scratch simple protocol replacing complicated Winkeyer
protocol ([version 1](https://k1el.tripod.com/files/Winkey10.pdf), [version 2](https://k1el.tripod.com/WinkeyUSBman.pdf),
[version 3](https://hamcrafters2.com/files/WK3_Datasheet_v1.3.pdf)), until I realized it
prevented me from using Spider Keyer with all contest programs except [HamRacer](http://ok1fig.nagano.cz/HamRacer/HamRacer.htm)
(also by OK1FIG, unsurprisingly). And then I also realized I didn't like that speed
control potentiometer was the only option. Then we have had some very useful exchange
of ideas and mutual inspiration with Peter that resulted in further code improvements
on both sides. However, every author usually always thinks their code is the best
(or best suited to their needs), so I chose to rename my version of Spider firmware
to "Spider Challenger" to signal out my conviction that some of my ideas might be more
practical or more useful.

### Making the code even more readable and transparent

In the end I rewrote most of the Spider Challenger code, reducing its size by some 60% vs K3NG code and by some 20% or more vs OK1FIG.
My main approach was to separate different functional units into classes and encapsulate most of the functionality into their member
variables and methods.

### The dream of Winkeyer compatibility and the looming total rework

After some time I wanted to add Winkeyer protocol in order to make Spider Challenger
interoperable with existing contest programs. Then I realized there were still some
"spaghetti" left in the code and came to inevitable decision to rewrite everything from
scratch.

Since this was the second attempt to rewrite keyer code and I still consider it a kind of
"glove thrown" to both K3NG and OK1FIG (with all my friendly respect, however!), I named
this project ***Challenger 2***.

## Firmware architecture

I took some general inspiration from *nodejs*, namely its event loop. By its nature, some
keyer code must be asynchronous (paddle state, serial communication) while other parts
must adhere to strict, synchronous timing. As most of the code features only consume
several tens of microseconds but the actual mark/space timing is in milliseconds (more
precisely: tens of milliseconds, e.g. 40 ms unit time for 30 WPM), each functional
component can check time and carry out its current action accordingly in every loop
iteration without causing any congestions.

It is unnecessary to mention that the idea of event loop is a perfect fit with the basic
Arduino concept of organizing all the code into two main functions, `setup()` and `loop()`.

### Main Functional Components

Main program setup and the main event loop are implemented in Arduino framework functions
`setup()` and `loop()`, respectively.

Functional components are implemented as C++ classes and realized as singleton instances in the following structure:

* **Keying Interface**: everything concerning keying output (KEY and PTT lines), sidetone handling and timing.
* **Paddle Interface**: everything related to paddle input and buffer for Iambic and Ultimatic keying, it feeds elements to be sent to Keying Interface
  when operator keyes with paddles.
* **Speed Control**: everything related to speed control by rotary encoder or potentiometer. In this project rotary encoder is a priority, potentiometer will be implemented at a later stage, during completion of Winkeyer protocol.
* **Morse Engine**: morse code codec, responsible for translation of ASCII characters into morse code sequence and vice versa,
  also responsible to feed elements to Keying Interface when sending text buffer characters.
* **Text Buffer**: circular buffer that receives characters from Protocol (see below) and provides characters from buffer to Morse Engine on request.
* **Protocol**: reads data stream from serial port, writes data to serial port, fills Text
Buffer when necessary, decodes received commands and executes the respective actions.

## Source code conventions

After the horrible experience of configuring K3NG source code (dozens of `#define`'s in
files like `keyer_pins.h`, `keyer_features_and_options.h` and `keyer_settings.h`, all
unrelated things mixed up in a jungle of `#define`'s and comments) and a number or trials
and errors before I hit the correct parameter, I finally understood that each separable
functional component of code must have its own configuration concentrated in one place only.

Therefore the general source code structure of each software component consists
of three files:

* Header file `include/config_{component}.h` contains *only* configurable constants as
`#define` macros. Under normal circumstances this is the only file that should be edited
by end user to fit Challenger source code to different hardware setup. Some components do
not need this (if there are no pins involved, there is usually nothing to customize).
* Header file `include/{component}.h` is the usual C++ header file. Each component header
file contains `extern` declaration of its respective singleton instance to ensure access
in the main event loop.
* Implementation file `src/{component}.cpp` contains implementation of functional
component methods and declaration of the respective singleton instance.

| Functional Component | Component File Name | actual file names | note |
|--|--|--|--|
| **Keyer Interface**| keying | `config_keying.h`, `keying.h`, `keying.cpp` |
| **Paddle Interface**| keying | `config_paddle.h`, `paddle.h`, `paddle.cpp` |
| **Speed Control** | speedcontrol | `config_speedcontrol.h`, `speedcontrol.h`, `speedcontrol.cpp` | *Hardware variants for rotary encoder or potentiometer to be implemented as  derived classes inheriting from common* `SpeedController` *base class* |
| **Morse Engine**| morse | `morse.h`, `morse.cpp` | *Not customizable by end user (no hardware dependencies)* |
| **Text Buffer** | keying | `buffer.h`, `buffer.cpp` | *Not customizable by end user (no hardware dependencies)* |
| **Protocol** | protocol | `config_keying.h`, `keying.h`, `keying.cpp` | *Concurrent protocols may be implemented as derived classes from* `Protocol` *base class (decision pending)* |

Have a look at [milestones](https://github.com/radio-miskovice/Challenger2/blob/main/doc/milestones.md)
