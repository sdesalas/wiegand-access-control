#define SETTINGS_FILE_COUNT 4
#define SETTINGS_DEFAULTS_TEMPLATE "/defaults/#.json"
#define SETTINGS_FILE_TEMPLATE  "/settings/#.json"

// "access" options
String setting_access_master_key = "";
String setting_access_users = "";
boolean setting_access_swapdoors = false;
// "wifi" options
String setting_wifi_ap_ssid = "";
String setting_wifi_ap_password = "";
boolean setting_wifi_ap_hidden = false;
String setting_wifi_ssid = "";
String setting_wifi_password = "";
boolean setting_wifi_hidden = false;
// "telemetry" options
int setting_telemetry_frequency = 0;
String setting_telemetry_url = "";
String setting_telemetry_header = "";
// "reboot" options 
String setting_reboot_key = "";

void Settings_init() {
  // Initialize the settings
  Settings_load("access");
  Settings_load("wifi");
  Settings_load("telemetry");
  Settings_load("reboot");
  Serial.print("The AP SSID is ");
  Serial.println(setting_wifi_ssid);
}

void Settings_load(const char* file) {
    String settingsPath = SETTINGS_FILE_TEMPLATE;
    settingsPath.replace("#", file);
    JsonDocument doc = FS_readJson(settingsPath.c_str());
    JsonObject setting = doc.as<JsonObject>();
    if (strcmp(file, "access") == 0) {
      setting_access_master_key = setting["master_key"] | false;
      setting_access_users = setting["users"] | "";
      setting_access_swapdoors = setting["swapdoors"] | false;
    }
    if (strcmp(file, "wifi") == 0) {
      setting_wifi_ap_ssid = setting["ap_ssid"] | "";
      setting_wifi_ap_password = setting["ap_password"] | "";
      setting_wifi_ap_hidden = setting["ap_hidden"] | false;
      setting_wifi_ssid = setting["ssid"] | "";
      setting_wifi_password = setting["password"] | "";
    }
    if (strcmp(file, "telemetry") == 0) {
      setting_telemetry_frequency = setting["frequency"] | 0;
      setting_telemetry_url = setting["url"] | "";
      setting_telemetry_header = setting["header"] | "";
    }
    if (strcmp(file, "reboot") == 0) {
      setting_reboot_key = setting["key"] | "";
    }
}

bool Settings_save(JsonObjectConst input, const char* file) {
    // Get the defaults, then overwrite with input
    String defaultsPath = SETTINGS_DEFAULTS_TEMPLATE;
    defaultsPath.replace("#", file);
    JsonDocument doc = FS_readJson(defaultsPath.c_str());
    JsonObject settings = doc.as<JsonObject>();
    for (JsonPairConst kv: input) {
      settings[kv.key()] = kv.value();
    }
    // Save settings to file
    String settingsPath = SETTINGS_FILE_TEMPLATE;
    settingsPath.replace("#", file);
    return FS_writeJson(settingsPath.c_str(), settings) > 0;
}

void Settings_reset() {
  String files[] = { "access", "wifi" , "telemetry", "reboot" };
  int size = sizeof(files)/sizeof(*files);
  for (int i = 0; i < size; i++) {
    String defaultsPath = SETTINGS_DEFAULTS_TEMPLATE;
    String settingsPath = SETTINGS_FILE_TEMPLATE;
    defaultsPath.replace("#", files[i]);
    settingsPath.replace("#", files[i]);
    Serial.print("Resetting ");
    Serial.println(settingsPath);
    File from = LittleFS.open(defaultsPath, "r");
    File to = LittleFS.open(settingsPath, "w");
    while(from.available()) {
      char c = from.read();
      to.write(c);
    }
    from.close();
    to.close();
  }
}
