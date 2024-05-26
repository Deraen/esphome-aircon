#include "esphome.h"
#include "IRremoteESP8266.h"
#include "IRsend.h"
#include "ir_Electra.h"

const uint16_t kIrLed = 17;
IRElectraAc ac(kIrLed);

class ElectraAc : public Component, public Climate, public CustomAPIDevice
{
private:
  sensor::Sensor* temp_sensor{nullptr};

public:
  void set_temp_sensor(sensor::Sensor *sensor) {
    this->temp_sensor = sensor;
  }

  void setMode(ClimateMode climateMode) {
    switch (climateMode)
    {
    case CLIMATE_MODE_HEAT:
      ac.setMode(kElectraAcHeat);
      ac.on();
      break;
    case CLIMATE_MODE_COOL:
      ac.setMode(kElectraAcCool);
      ac.on();
      break;
    case CLIMATE_MODE_HEAT_COOL:
      ac.setMode(kElectraAcAuto);
      ac.on();
      break;
    case CLIMATE_MODE_DRY:
      ac.setMode(kElectraAcDry);
      ac.on();
      break;
    case CLIMATE_MODE_FAN_ONLY:
      ac.setMode(kElectraAcFan);
      ac.on();
      break;
    }
  }

  void setFan(optional<ClimateFanMode> mFanMode) {
    if (!mFanMode.has_value()) return;

    ClimateFanMode fanMode = mFanMode.value();
    switch (fanMode)
    {
    case CLIMATE_FAN_AUTO:
      ac.setFan(kElectraAcFanAuto);
      ac.setTurbo(false);
      break;
    case CLIMATE_FAN_LOW:
      ac.setFan(kElectraAcFanLow);
      ac.setTurbo(false);
      break;
    case CLIMATE_FAN_MEDIUM:
      ac.setFan(kElectraAcFanMed);
      ac.setTurbo(false);
      break;
    case CLIMATE_FAN_HIGH:
      ac.setFan(kElectraAcFanHigh);
      ac.setTurbo(false);
      break;
    }
  }

  void setSwing(ClimateSwingMode swingMode) {
    switch (swingMode)
    {
    case CLIMATE_SWING_OFF:
      ac.setSwingV(false);
      break;
    case CLIMATE_SWING_VERTICAL:
      ac.setSwingV(true);
      break;
    }
  }

  void setup() override {
    register_service(&ElectraAc::resend, "resend", {});

    if(this->temp_sensor != nullptr){
      this->temp_sensor->add_on_raw_state_callback([this](float temp) {
        update_temp(temp);
      });
    }

    auto restore = this->restore_state_();
    if (restore.has_value()) {
      ESP_LOGD("DEBUG", "Restoring stored state");
      restore->apply(this);
    } else {
      ESP_LOGD("DEBUG", "Initializing on default state");
      this->mode = climate::CLIMATE_MODE_COOL;
      this->target_temperature = 21;
      this->fan_mode = climate::CLIMATE_FAN_AUTO;
      this->swing_mode = climate::CLIMATE_SWING_OFF;
    }

    ac.begin();
    setMode(this->mode);
    ac.setTemp(this->target_temperature);
    setFan(this->fan_mode);
    setSwing(this->swing_mode);

    ac.setSwingV(kElectraAcSwingOn);

    ESP_LOGD("DEBUG", "A/C remote is in the following state:");
    ESP_LOGD("DEBUG", "  %s\n", ac.toString().c_str());
  }

  void update_temp(float temp) {
    if(isnan(temp)){
      return;
    }

    this->current_temperature = temp;
    this->publish_state();
  }

  climate::ClimateTraits traits()
  {
    auto traits = climate::ClimateTraits();

    traits.set_supports_current_temperature(true);
    traits.set_supports_two_point_target_temperature(false);
    traits.set_visual_min_temperature(16);
    traits.set_visual_max_temperature(32);
    traits.set_visual_current_temperature_step(0.1);
    traits.set_visual_target_temperature_step(1);

    std::set<ClimateMode> climateModes;
    climateModes.insert(CLIMATE_MODE_OFF);
    climateModes.insert(CLIMATE_MODE_HEAT_COOL);
    climateModes.insert(CLIMATE_MODE_COOL);
    climateModes.insert(CLIMATE_MODE_HEAT);
    climateModes.insert(CLIMATE_MODE_DRY);
    climateModes.insert(CLIMATE_MODE_FAN_ONLY);
    traits.set_supported_modes(climateModes);

    std::set<ClimateFanMode> climateFanModes; 
    climateFanModes.insert(CLIMATE_FAN_AUTO);
    climateFanModes.insert(CLIMATE_FAN_LOW);
    climateFanModes.insert(CLIMATE_FAN_MEDIUM);
    climateFanModes.insert(CLIMATE_FAN_HIGH);
    traits.set_supported_fan_modes(climateFanModes);

    std::set<ClimateSwingMode> climateSwingModes;
    climateSwingModes.insert(CLIMATE_SWING_OFF);
    climateSwingModes.insert(CLIMATE_SWING_VERTICAL);
    traits.set_supported_swing_modes(climateSwingModes);

    return traits;
  }

  void control(const ClimateCall &call) override
  {
    if (call.get_mode().has_value())
    {
      ClimateMode climateMode = *call.get_mode();
      setMode(climateMode);
      this->mode = climateMode;
      this->publish_state();
    }

    if (call.get_target_temperature().has_value()) {
      float temp = *call.get_target_temperature();
      ac.setTemp(temp);
      this->target_temperature = temp;
      this->publish_state();
    }

    if (call.get_fan_mode().has_value()) {
      ClimateFanMode fanMode = *call.get_fan_mode();
      setFan(fanMode);
      this->fan_mode = fanMode;
      this->publish_state();
    }

    if (call.get_swing_mode().has_value()) {
      ClimateSwingMode swingMode = *call.get_swing_mode();
      setSwing(swingMode);
      this->swing_mode = swingMode;
      this->publish_state();
    }

    // Send and retry one extra time
    ac.send(1);
  }

  void resend()
  {
    ESP_LOGD("DEBUG", "Resending the current state:");
    ESP_LOGD("DEBUG", "  %s\n", ac.toString().c_str());
    ac.send(1);
  }
};
