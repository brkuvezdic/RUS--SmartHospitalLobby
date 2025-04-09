Power Management System with Arduino

📝 Description
This Arduino project demonstrates energy-saving techniques using different sleep modes. The system flashes an LED for 3 seconds, then enters a low-power sleep state. Wake-up can be triggered either by a button press (external interrupt) or automatically after a time interval using Timer1 (~8 seconds).

✨ Features
Three different sleep modes:

IDLE - Minimal power saving

POWER_DOWN - Maximum power saving

POWER_SAVE - Balanced power saving

Automatic sleep mode rotation every 3 cycles

Wake-up sources:

External interrupt (button press)

Timer1 overflow interrupt (~8 seconds)

Serial monitoring of system states

Power optimization by disabling unused peripherals

🛠 Hardware Setup
Components
Arduino Uno

LED (connected to pin 13)

Push button (connected to pin 2 with internal pull-up)

Circuit Diagram
Copy
Arduino Uno:
  - Pin 13 → LED → GND
  - Pin 2 → Button → GND
📚 Libraries Used
<avr/sleep.h> - For sleep modes

<avr/interrupt.h> - For interrupt handling

<avr/power.h> - For power management

🔧 How It Works
Active State:

LED blinks for 3 seconds (3x 500ms on/off)

System prints status information to Serial Monitor

Sleep State:

Enters one of three sleep modes (rotates every 3 cycles)

Disables unnecessary peripherals for additional power saving

Can be woken by:

Button press (external interrupt on falling edge)

Timer1 overflow (~8 seconds)

Wake-up:

System detects wake source

Re-enables peripherals

Reports wake-up reason via Serial Monitor

Repeats the cycle

📊 Sleep Modes Comparison
Mode	Power Saving	Wake-up Sources	Peripherals Disabled
IDLE	Minimal	Any interrupt	None
POWER_DOWN	Maximum	External interrupts/reset	All non-essential
POWER_SAVE	Balanced	Timer1, external interrupts	ADC, SPI, TWI
🚀 Getting Started
Hardware Setup:

Connect LED to pin 13

Connect button to pin 2 (with internal pull-up)

Upload Code:

Compile and upload the sketch to your Arduino

Monitor:

Open Serial Monitor at 9600 baud to see system messages

📝 Notes
Button uses internal pull-up resistor (active LOW)

Timer1 is configured for ~8 second wake-up intervals

System automatically rotates sleep modes every 3 cycles

For maximum power saving, use POWER_DOWN mode with minimal peripherals
