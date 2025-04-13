#include "MultiWire.h"

#define MOTOR_BRAKE_MOD 0x7D
#define MOTOR_SET_POWER_C1 0x40
#define MOTOR_SET_POWER_C2 0x41

#define EXPANSION_RESET 0x27
#define EXPANSION_ENABLE 0x25
#define EXPANSION_RESET_TIME 1000

#define REQUEST_MOTOR_CURRENT_C1 0x54
#define REQUEST_MOTOR_CURRENT_C2 0x55

#define REQUEST_MOTOR_POSITION_C1 0x49
#define REQUEST_MOTOR_POSITION_C2 0x4A

#define MOTOR_POSITION_RESET_C1 0x4C
#define MOTOR_POSITION_RESET_C2 0x4D

enum ZeroPowerBehavior
{
    FLOAT = false,
    BRAKE = true
};

enum Direction
{
    FORWARD = false,
    REVERSE = true
};

class DcExpansion
{
private:
    bool _enabled;

    uint32_t _resetTime = 0;

public:
    DcExpansion(const uint8_t addr, const IWire *wir) : address(addr), wire(wir)
    {
        _enabled = false;
    }

    const uint8_t address;
    const IWire *wire;

    void begin()
    {
        wire->write8(address, EXPANSION_RESET);

        _resetTime = millis();
    }

    void enable()
    {
        if (_enabled)
            return;

        while (millis() - _resetTime < EXPANSION_RESET_TIME);

        wire->write8(address, EXPANSION_ENABLE);

        _enabled = true;
    }

    bool isEnabled()
    {
        return _enabled;
    }
};

class DcMotorEx
{
    DcExpansion *_expansion;

    uint8_t _channel;

    bool _direction, _zeroPowerBehavior;

    int8_t _lastPower = 0;
    float _lastFloatPower = 0.0f;

    float _maxPower = 1.0f;

    int32_t _encoderResetPos = 0;

    float _currentErr = -0.18;

    int32_t getRawCurrentPosition()
    {
        _expansion->wire->write8(_expansion->address, _channel == 1 ? REQUEST_MOTOR_POSITION_C1 : REQUEST_MOTOR_POSITION_C2);

        _expansion->wire->requestFrom(_expansion->address, 4);
        uint8_t buf[4];
        _expansion->wire->readBytes(buf, 4);

        uint32_t ticks = buf[0];
        ticks = (ticks * 256) + buf[1];
        ticks = (ticks * 256) + buf[2];
        ticks = (ticks * 256) + buf[3];

        return (int32_t)(ticks) * (_direction ? -1 : 1);
    }

public:
    DcMotorEx(DcExpansion *expansion, uint8_t channel, bool zeroPowerBehavior = true, bool direction = false, float maxPower = 1.0f, float currentErr = -0.18)
    {
        _expansion = expansion;
        _channel = channel;
        _zeroPowerBehavior = zeroPowerBehavior;
        _direction = direction;
        _maxPower = maxPower;
        _currentErr = currentErr;
    }

    void begin()
    {
        if (!_expansion->isEnabled())
            _expansion->enable();

        resetEncoder();
    }

    void setMaxPower(float maxPower){
        _maxPower = maxPower;
        
        setPower(_lastFloatPower);
    }

    void setZeroPowerBehavior(bool behavior)
    {
        _zeroPowerBehavior = behavior;
        
        setPower(_lastFloatPower);
    }

    void setDirection(bool direction)
    {
        _direction = direction;

        setPower(_lastFloatPower);
    }

    float getPower(){
        return _lastFloatPower;
    }

    void setPower(float power)
    {
        _lastFloatPower = power;

        int8_t intMaxPower = 100 * _maxPower;

        int8_t intPower = min(intMaxPower, max(-intMaxPower, power * (_direction ? -100.0f : 100.0f)));

        if (intPower == 0 && _zeroPowerBehavior)
            intPower = MOTOR_BRAKE_MOD;

        if (intPower != _lastPower)
        {
            _lastPower = intPower;

            _expansion->wire->write2x8(_expansion->address, _channel == 1 ? MOTOR_SET_POWER_C1 : MOTOR_SET_POWER_C2, (uint8_t)intPower);
        }
    }

    float getCurrent()
    {
        _expansion->wire->write8(_expansion->address, _channel == 1 ? REQUEST_MOTOR_CURRENT_C1 : REQUEST_MOTOR_CURRENT_C2);

        _expansion->wire->requestFrom(_expansion->address, (uint8_t)2);
        uint8_t buf[2];
        _expansion->wire->readBytes(buf, 2);

        return ((uint16_t)(buf[0] * 256 + buf[1]) * (_direction ? -1 : 1)) / 1000.0f + _currentErr;
    }

    int32_t getCurrentPosition(){
        return getRawCurrentPosition() - _encoderResetPos;
    }

    void resetEncoder()
    {
        _expansion->wire->write8(_expansion->address, _channel == 1 ? MOTOR_POSITION_RESET_C1 : MOTOR_POSITION_RESET_C2);

        _encoderResetPos = 0;
    }

    void softwareEncoderReset(){
        _encoderResetPos = getRawCurrentPosition();
    }
};