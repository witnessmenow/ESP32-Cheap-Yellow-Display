using System;
using System.Device.Pwm;
using System.Diagnostics;
using System.Threading;
using nanoFramework.Hardware.Esp32;

//Supports top level statements
Debug.WriteLine("Hello from nanoFramework!");

bool goingUp = true;
float dutyCycle = .00f;

Configuration.SetPinFunction(17, DeviceFunction.PWM2);
PwmChannel pwmPin = PwmChannel.CreateFromPin(17, 40000, 0);

// Start the PWM
pwmPin.Start();

while (true)
{
    if (goingUp)
    {
        // slowly increase light intensity
        dutyCycle += 0.05f;

        // change direction if reaching maximum duty cycle (100%)
        if (dutyCycle > .95)
            goingUp = !goingUp;
    }
    else
    {
        // slowly decrease light intensity
        dutyCycle -= 0.05f;

        // change direction if reaching minimum duty cycle (0%)
        if (dutyCycle < 0.10)
            goingUp = !goingUp;
    }

    // update duty cycle
    pwmPin.DutyCycle = dutyCycle;

    Thread.Sleep(50);
}

// Can't deploy the code if unreachable statements exist hence these are commented out
// Stop the PWM:
//pwmPin.Stop();

//Thread.Sleep(Timeout.Infinite);
