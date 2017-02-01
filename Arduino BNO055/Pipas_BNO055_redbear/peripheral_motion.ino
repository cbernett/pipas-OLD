#include <BLE_API.h>

#define TXRX_BUF_LEN    20
#define LOCAL_NAME      "Pipas"
#define SCAN_NAME       "Scan Pipas!"


// note that RBL defaults to a relatively slow configuration
// for full speed make it go faster with a tweak to the core header config:
// ~/Library/Arduino15/packages/RedBearLab/hardware/nRF51822/1.0.5/cores/RBL_nRF51822/nRF51822-master/source/projectconfig.h
// http://www.corbinstreehouse.com/blog/2015/12/tricks-for-fast-bluetooth-le-data-transfers/
// https://devzone.nordicsemi.com/question/1741/dealing-large-data-packets-through-ble/


// this is the payload; must be < 21 bytes
typedef struct {
  uint32_t timestamp;
  // note that these are in half-float (aka sfloat) format
  struct {
    uint16_t w;
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } quat;
  struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } pos;
} motion_update_t;


BLE                                       ble;
Ticker                                    ticker;

static uint32_t timestamp = 0;

// The IMU service
static const uint8_t imu_service_uuid[] =     { 0x0C, 0x33, 0x04, 0xF4, 0x5F, 0x4C, 0x43, 0xB3, 0xB7, 0xAF, 0x50, 0xCF, 0xBA, 0x8C, 0x8F, 0x36 };
// this reversed version is used for advertising data (little-endian per BLE spec, but ARM is big-endian)
static const uint8_t imu_service_uuid_rev[] = { 0x36, 0x8F, 0x8C, 0xBA, 0xCF, 0x50, 0xAF, 0xB7, 0xB3, 0x43, 0x4C, 0x5F, 0xF4, 0x04, 0x33, 0x0C };
static const uint8_t imu_service_imu_characteristic_uuid[] = { 0x7A, 0xB5, 0xCA, 0x03, 0xCC, 0x7B, 0x4D, 0x2C, 0x89, 0xEF, 0x9F, 0x07, 0x4C, 0xEA, 0x8F, 0xE6 };


GattCharacteristic  imu_characteristic(imu_service_imu_characteristic_uuid, NULL, 0, sizeof(motion_update_t), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

GattCharacteristic *characteristics[] = {&imu_characteristic};

GattService         imuService(imu_service_uuid, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));


Gap::Handle_t activeConnection = 0;

void CheckError(ble_error_t err, const char *s) {
  if(err != BLE_ERROR_NONE) {
    Serial.print("Error: ");
    Serial.print(err);
    Serial.print(": ");
    Serial.println(s);
  }
}

void connectionCallBack(const Gap::ConnectionCallbackParams_t *params)
{
    Serial.println("Got connection");
    ble.stopAdvertising();
    activeConnection = params->handle;
}

void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params)
{
    Serial.println("Disconnected ");
    Serial.println("Restart advertising ");
    ble.startAdvertising();
    activeConnection = 0;
}

void m_100ms_handle(void)
{
//    Serial.println("100ms Loop ");

  if(!ble.getGapState().connected) {
//    Serial.println("not connected");
    return;
  }

  // TODO how do I know if a connected client has requested notifications?

    const GattAttribute::Handle_t& handle = imu_characteristic.getValueHandle();
    uint16_t payload[2 + 4 + 3]; // first 4 values are quaternion, second 3 are vec position
    
    int i = 0;
    ++timestamp;
    payload[i++] = (uint16_t) timestamp & 0xFFFF;
    payload[i++] = (uint16_t) (timestamp >> 16) & 0xFFFF;
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gQuat.w() );
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gQuat.x() );
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gQuat.y() );
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gQuat.z() );
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gLinearAcceleration.x() );
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gLinearAcceleration.y() );
    payload[i++] = (uint16_t) floatToHalfFloat( (float) gLinearAcceleration.z() );

    ble_error_t err;
//    if(activeConnection) {
//      err = ble.gattServer().write(activeConnection, handle, (uint8_t *)payload, sizeof(payload));
//    } else {
//      err = ble.gattServer().write(handle, (uint8_t *)payload, sizeof(payload));
//    }

    bool enabled;
    err = ble.gattServer().areUpdatesEnabled(imu_characteristic, &enabled);
    if(err != BLE_ERROR_NONE) {
      Serial.print("areUpdatesEnabled error: "); Serial.println(err);
      return;
    }
    if(!enabled) {
      Serial.println("no updates");
      return;
    }

    err = ble.gattServer().write(handle, (uint8_t *)payload, sizeof(payload));

    if(err != BLE_ERROR_NONE) {
      Serial.print("size: "); Serial.print(sizeof(payload)); Serial.print(" ");
      Serial.print("error: ");
      Serial.println(err);
    } else {
//      Serial.println("Sent BLE packet");
      Serial.print("Sent ");
      Serial.print(timestamp); Serial.print(" - ");
      Serial.print(gQuat.w()); Serial.print(" ");
      Serial.print(gQuat.x()); Serial.print(" ");
      Serial.print(gQuat.y()); Serial.print(" ");
      Serial.print(gQuat.z()); Serial.print(" ");
      Serial.println();
    }
}


void peripheral_motion_setup() {

    ticker.attach(m_100ms_handle, 0.1);

    // put your setup code here, to run once
    Serial.begin(9600);

    Serial.println("Start ");

    ble_error_t err;
    
    ble.init();
    ble.gap().onConnection(connectionCallBack);
    ble.gap().onDisconnection(disconnectionCallBack);


    //*

    // setup adv_data and srp_data
    
    err = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    CheckError(err, "adv flags");
    err = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                                     (const uint8_t *)LOCAL_NAME, sizeof(LOCAL_NAME) - 1);
    CheckError(err, "adv localname");
    err = ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                     (const uint8_t *)imu_service_uuid_rev, sizeof(imu_service_uuid_rev));
//    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SERVICE_DATA, (const uint8_t *)"Oy", sizeof("Oy") - 1);
    CheckError(err, "adv service ids");
    
    /*
    err = ble.gap().accumulateScanResponse(GapAdvertisingData::COMPLETE_LOCAL_NAME,
                              (const uint8_t *)SCAN_NAME, sizeof(SCAN_NAME) - 1);
    CheckError(err, "scan locla name");
    err = ble.gap().accumulateScanResponse(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                              (const uint8_t *)service1_uuid_rev, sizeof(service1_uuid_rev));
    CheckError(err, "scan service ids");
    //*/

    // set adv_type
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    // add service
    ble.gattServer().addService(imuService);
    // set device name
    ble.gap().setDeviceName((const uint8_t *)"Pipeiro");
    // set tx power,valid values are -40, -20, -16, -12, -8, -4, 0, 4
    ble.setTxPower(4);

    // set adv_interval, 100ms in multiples of 0.625ms.
//    ble.setAdvertisingInterval(160);
    ble.setAdvertisingInterval(10);
    // set adv_timeout, in seconds
    ble.setAdvertisingTimeout(0);
    // ger BLE stack version
    Serial.println( ble.getVersion() );
    // start advertising
    ble.startAdvertising();

    Serial.println("start advertising ");
}

void peripheral_motion_loop() {
    ble.waitForEvent();
}

