#include "REVSDK.h"

void runOpMode(){
  DcMotorEx motor(&motors, 2); //получение мотора на канале 2
  motor.begin();

  waitForStart();

  motor.resetEncoder();

  while(opModeIsActive()){
    motor.setPower(0.5);

    telemetryAddData("power", motor.getPower());

    telemetryAddData("current", motor.getCurrent());

    telemetryAddData("currentPos", motor.getCurrentPosition());

    telemetryUpdate();
  }
}