#include <lvgl.h>
#include <TFT_eSPI.h>

#define LVGL_TICK_PERIOD 5

TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

#include "rpcWiFi.h"
#include <ArduinoJson.h>
 
const char* ssid = "********";
const char* password =  "********";
const char* city_name = "shenzhen";



/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static int32_t last_diff = 0;
    int32_t diff = 0; /* Dummy - no movement */
    int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */
  
    data->enc_diff = diff - last_diff;;
    data->state = btn_state;
  
    last_diff = diff;
  
    return false;
}



static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint16_t c;
  
    tft.startWrite(); /* Start new TFT transaction */
    tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
    for (int y = area->y1; y <= area->y2; y++) {
        for (int x = area->x1; x <= area->x2; x++) {
            c = color_p->full;
            tft.writeColor(c, 1);
            color_p++;
        }
    }
    tft.endWrite(); /* terminate TFT transaction */
    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

char * get_weather_type(unsigned int i)
{
    if (i == 0)
    {
        return "sunny";
    } 
    else if (i< 10)
    {
        return "cloudy";
    }
    else
    {
        return "rainy";
    }
}

void make_table(lv_obj_t * parent, JsonObject &obj_1, JsonObject &obj_2)
{

    lv_obj_t * table = lv_table_create(parent, NULL);
    lv_obj_clean_style_list(table, LV_TABLE_PART_BG);
    lv_obj_set_click(table, false);

    lv_table_set_col_cnt(table, 2);
    lv_table_set_row_cnt(table, 4);
    
    lv_coord_t w = lv_page_get_width_fit(parent) - 10;
    lv_table_set_col_width(table, 0, 3 * w / 10);
    lv_table_set_col_width(table, 1, w  / 4);
          
    lv_obj_set_pos(table, 0, 0);
    lv_obj_set_size(table, 60, 40);

    // get today's weather info
    int code = obj_1["results"][0]["daily"][0]["day_code"];    
    int temp_high = obj_1["results"][0]["daily"][0]["high"];
    int temp_low = obj_1["results"][0]["daily"][0]["low"];
    int humidity = obj_1["results"][0]["daily"][0]["humidity"];
    int wind = obj_1["results"][0]["daily"][0]["wind_scale"];    

    int aqi = obj_2["results"][0]["air"]["city"]["aqi"]; 


    /*Fill the first column*/
    lv_table_set_cell_value(table, 0, 0, "Today");
    lv_table_set_cell_value(table, 1, 0, "TEMP(C)");
    lv_table_set_cell_value(table, 2, 0, "RH(%)");
    lv_table_set_cell_value(table, 3, 0, "AQI");

    /*Fill the second column*/
    char str_hum[20];
    char str_aqi[20];
    char str_temp[20];
    char str_high[20];
    char str_low[20];
    itoa(humidity, str_hum, 10);
    itoa(aqi, str_aqi, 10);
    itoa(temp_high, str_high, 10);
    itoa(temp_low, str_low, 10);
    sprintf(str_temp, "%s~%s", str_low, str_high);
    lv_table_set_cell_value(table, 0, 1, get_weather_type(code));
    lv_table_set_cell_value(table, 1, 1, str_temp);
    lv_table_set_cell_value(table, 2, 1, str_hum);
    lv_table_set_cell_value(table, 3, 1, str_aqi);
}


void update_display(JsonObject &obj_1, JsonObject &obj_2)
{

    // today
    const char * date_name = obj_1["results"][0]["daily"][0]["date"];

    // day 1
    const char * date_day1 = obj_1["results"][0]["daily"][1]["date"];
    int high_day1 = obj_1["results"][0]["daily"][1]["high"];
    int low_day1 = obj_1["results"][0]["daily"][1]["low"];
    int code_day1 = obj_1["results"][0]["daily"][1]["day_code"];

    // day 2
    const char * date_day2 = obj_1["results"][0]["daily"][2]["date"];
    int high_day2 = obj_1["results"][0]["daily"][2]["high"];
    int low_day2 = obj_1["results"][0]["daily"][2]["low"];
    int code_day2 = obj_1["results"][0]["daily"][2]["day_code"];

    // day 3
    const char * date_day3 = obj_1["results"][0]["daily"][3]["date"];
    int high_day3 = obj_1["results"][0]["daily"][3]["high"];
    int low_day3 = obj_1["results"][0]["daily"][3]["low"];
    int code_day3 = obj_1["results"][0]["daily"][3]["day_code"];
    
    // display settings
    lv_obj_clean(lv_scr_act());

    lv_obj_t* page = lv_page_create(lv_scr_act(), NULL);
    lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
    
    // LOGO: Wio Weather
    lv_obj_t * logo = lv_btn_create(page, NULL);
    lv_btn_set_fit(logo, LV_FIT_TIGHT);
    lv_obj_align(logo, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

    lv_obj_t * label = lv_label_create(logo, NULL);
    lv_label_set_text(label, "Wio\nWeather");

    // date
    lv_obj_t * date = lv_label_create(page, NULL);
    lv_label_set_text_fmt(date, "%s", date_name);
    lv_obj_set_pos(date, 200, 65);

    // city
    lv_obj_t * city = lv_label_create(page, NULL);
    lv_label_set_text_fmt(city, "%s", city_name);
    lv_obj_set_pos(city, 200, 90);    


    // display infor the the comming two days
    lv_obj_t * title_day1 = lv_label_create(page, NULL);
    lv_label_set_text(title_day1, date_day1);
    lv_obj_set_pos(title_day1, 10, 170);

    lv_obj_t * info_day1 = lv_label_create(page, NULL);
    lv_label_set_text(info_day1, get_weather_type(code_day2));
    lv_obj_set_pos(info_day1, 15, 190);    

    lv_obj_t * temp_day1 = lv_label_create(page, NULL);
    lv_label_set_text_fmt(temp_day1, "%d~%d", low_day1, high_day1);
    lv_obj_set_pos(temp_day1, 15, 210);     


    lv_obj_t * title_day2 = lv_label_create(page, NULL);
    lv_label_set_text(title_day2, date_day2);
    lv_obj_set_pos(title_day2, 110, 170);

    lv_obj_t * info_day2 = lv_label_create(page, NULL);
    lv_label_set_text(info_day2, get_weather_type(code_day2));
    lv_obj_set_pos(info_day2, 115, 190);  

    lv_obj_t * temp_day2 = lv_label_create(page, NULL);
    lv_label_set_text_fmt(temp_day2, "%d~%d", low_day2, high_day2);
    lv_obj_set_pos(temp_day2, 115, 210);      


    lv_obj_t * title_day3 = lv_label_create(page, NULL);
    lv_label_set_text(title_day3, date_day3);
    lv_obj_set_pos(title_day3, 210, 170);

    lv_obj_t * info_day3 = lv_label_create(page, NULL);
    lv_label_set_text(info_day3, get_weather_type(code_day3));
    lv_obj_set_pos(info_day3, 215, 190);  

    lv_obj_t * temp_day3 = lv_label_create(page, NULL);
    lv_label_set_text_fmt(temp_day3, "%d~%d", low_day3, high_day3);
    lv_obj_set_pos(temp_day3, 215, 210);          
            
    make_table(page, obj_1, obj_2);
}


void setup() 
{
    Serial.begin(115200);
    // while(!Serial); // Wait to open Serial Monitor
    Serial.printf("RTL8720 Firmware Version: %s", rpc_system_version());
    Serial.println("");
    
    // turn on the backlight 
    // digitalWrite(LCD_BACKLIGHT, HIGH);    

    lv_init();
    
    tft.begin(); /* TFT init */
    tft.setRotation(3); /* Landscape orientation */
  
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  
    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the touch pad*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = read_encoder;
    lv_indev_drv_register(&indev_drv);
    
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.println("Connecting to WiFi..");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
        WiFi.begin(ssid, password);
    }
    Serial.println("Connected to the WiFi network");
    Serial.print("IP Address: ");
    Serial.println (WiFi.localIP()); // prints out the device's IP address
}


void loop() 
{
    const uint16_t port = 80; // Default port
    const char* host = "116.62.81.138";  // IP Address of api.seniverse.com

    Serial.print("Connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;

    if (!client.connect(host, port)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        return;
    } else {
        Serial.println("Connection succeed. ");
    }

    bool bSucceed = true;
    String line_daily = "";
    String line_air = "";
    
    // This will send a request of weather to the server
    client.println("GET /v3/weather/daily.json?key=your-api-key&location=shenzhen&language=zh-Hans&unit=c&start=0&days=5");
    client.println();

    int maxloops = 0;
    while (!client.available() && maxloops < 10) {
        maxloops++;
        delay(1); //delay 1 msec
    }
    if (client.available() > 0) 
    {
        //read back one line from the server
        line_daily = client.readString(); // Read from the server response
        // Proceed various line-endings
        line_daily.replace("\r\n", "\n");
        line_daily.replace('\r', '\n');
        line_daily.replace("\n", "\r\n");
        Serial.println(line_daily);
    } else {
        bSucceed = false;
        Serial.println("client.available() timed out ");
    }
    Serial.println("Closing connection.");
    client.stop();

    client.connect(host, port);
    // This will send a request of air quality to the server again
    client.println("GET /v3/air/now.json?key=your-api-key&location=shenzhen&language=zh-Hans&scope=city");
    client.println();

    maxloops = 0;
    while (!client.available() && maxloops < 10) {
        maxloops++;
        delay(1); //delay 1 msec
    }
    if (client.available() > 0) 
    {
        //read back one line from the server
        line_air = client.readString(); // Read from the server response
        // Proceed various line-endings
        line_air.replace("\r\n", "\n");
        line_air.replace('\r', '\n');
        line_air.replace("\n", "\r\n");
        Serial.println(line_air);
    } else {
        bSucceed = false;
        Serial.println("client.available() timed out ");
    }
    Serial.println("Closing connection.");
    client.stop();
    
    // if both requests are successful
    if (bSucceed == true)
    {
        DynamicJsonDocument doc_daily(4096);
        deserializeJson(doc_daily, line_daily);
        JsonObject obj_daily = doc_daily.as<JsonObject>();

        DynamicJsonDocument doc_air(4096);
        deserializeJson(doc_air, line_air);
        JsonObject obj_air = doc_air.as<JsonObject>();
    
        update_display(obj_daily, obj_air);    
    }
          
    lv_tick_handler();
    lv_task_handler(); /* let the GUI do its work */

    Serial.println("Waiting 10 seconds before restarting...");
    delay(10000);
}
