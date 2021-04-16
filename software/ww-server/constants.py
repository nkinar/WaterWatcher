# Constants for server to receive data
SERVPORT = 8000
HOST = "0.0.0.0"  # required to allow access from outside

# port to obtain data from Flask endpoint
WEB_GUI_PORT = 5000

# InfluxdB host and port
INFLUX_HOST = 'localhost'
INFLUX_PORT = 8086

# mongodb port
MONGO_PORT = 27017

# Turn this on to allow connections from localhost
ALLOW_LOCALHOST = True

TOKEN_MAX_CHARS = 10          # the token used as the key cannot be more than 10 characters
MAX_CHAR_LEN = 2**13          # ensure that the max character length is enough to receive the entire packet
DEFAULT_CONNECTIONS = 10      # the total number of connections to the server as the same time (adjust for more devices)
SER_NUMBER_MAX_CHARS = 50     # max number of characters in the serial number
TIME_STR_MAX_CHARS = 20       # max number of characters in the time string from the device
MAX_LOC_STRING = 30           # max number of characters in the name
SOCKET_TIMEOUT = 30.0         # wait time for socket timeout

NAN_STR = 'nan'
NAN_NO_DATA = '-999.0'

SENSOR_TOKEN_FIELD = 'token'
SENSOR_NUM_FIELD = 'num'
SENSOR_OUTPUT_FIELD = 'sensor_output'
OK_RESP = b'RECEIVED\r'
ERR_RESP = b'ERROR\r'
CR_STR = '\r'
LF_STR = '\n'
TEST_STR = 'TEST'

WW_SENSOR_DB_NAME = 'WW_SENSORS'
WW_SENSOR_COL_REG = 'registration'
OP_ADD = 'add'
OP_DEL = 'del'
OP_INFO = 'info'
TOKEN_CHARS = 10
TOKEN_STR = '[\d\w]{10}'
TOKEN_STR_SERIAL = '[\d\w]{20}'

# JSON data from sensor
a0_voltage_STR = 'a0_voltage'
a0_out_STR = 'a0_out'
a1_voltage_STR = 'a1_voltage'
a1_out_STR = 'a1_out'
a2_voltage_STR = 'a2_voltage'
a2_out_STR = 'a2_out'
temp_STR = 'temp'
temp0_STR = 'temp0'
temp0_out_STR = 'temp0_out'

serial_number_STR = 'serial_number'
serial_number_good_STR = 'serial_number_good'
battery_fault_STR = 'battery_fault'
battery_charging_STR = 'battery_charging'
rtc_temperature_STR = 'rtc_temperature'
start_time_str_STR = 'start_time_str'
end_time_str_STR = 'end_time_str'
btemperature_STR = 'btemperature'
bvoltage_STR = 'bvoltage'
bcurrent_STR = 'bcurrent'
bcapacity_STR = 'bcapacity'
uptime_STR = 'uptime'
name_STR = 'name'
latitude_STR = 'latitude'
longitude_STR = 'longitude'
speed_STR = 'speed'
altitude_STR = 'altitude'
height_STR = 'height'
lat_err_STR = 'lat_err'
long_err_STR = 'long_err'
alt_err_STR = 'alt_err'
gps_hours_STR = 'gps_hours'
gps_minutes_STR = 'gps_minutes'
gps_seconds_STR = 'gps_seconds'
gps_microseconds_STR = 'gps_microseconds'
gps_day_STR = 'gps_day'
gps_month_STR = 'gps_month'
gps_year_STR = 'gps_year'
gps_fix_quality_STR = 'gps_fix_quality'
gps_satellites_STR = 'gps_satellites'
gps_total_sats_STR = 'gps_total_sats'
gps_gdata_good_STR = 'gps_gdata_good'


# Schema for main data transport JSON


# Character used for CSV separation
TOKEN_CHAR_CSV = ','

# Data JSON schema
DATA_SCHEMA = {
    SENSOR_TOKEN_FIELD:
        {
            'type': 'string',
            'maxlength': TOKEN_MAX_CHARS
        },
    SENSOR_NUM_FIELD:
        {
            'type': 'integer',
            'coerce': int
        },

    # scientific collection strings
    a0_voltage_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    a0_out_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    a1_voltage_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    a1_out_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    a2_voltage_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    a2_out_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    temp0_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },
    temp0_out_STR: {
        'type': 'float',
        'coerce': float,
        'required': False
    },

    # TODO: add in the other temperature strings here if required
    # temp0, temp1, temp2,... as the water temperatures or 1-wire temperature sensors

    # system health strings
    serial_number_STR: {
            'type': 'string',
            'maxlength': SER_NUMBER_MAX_CHARS
    },
    serial_number_good_STR: {
        'type': 'integer',
        'coerce': int
    },
    battery_fault_STR: {
        'type': 'integer',
        'coerce': int
    },
    battery_charging_STR: {
        'type': 'integer',
        'coerce': int
    },
    rtc_temperature_STR: {
        'type': 'float',
        'coerce': float
    },
    start_time_str_STR: {
        'type': 'string',
        'maxlength': TIME_STR_MAX_CHARS,
    },
    end_time_str_STR: {
        'type': 'string',
        'maxlength': TIME_STR_MAX_CHARS,
    },
    btemperature_STR: {
        'type': 'float',
        'coerce': float
    },
    bvoltage_STR: {
        'type': 'float',
        'coerce': float
    },
    bcurrent_STR: {
        'type': 'float',
        'coerce': float
    },
    bcapacity_STR: {
        'type': 'float',
        'coerce': float
    },
    uptime_STR: {
        'type': 'integer',
        'coerce': int,
        'maxlength': TIME_STR_MAX_CHARS,
    },
    name_STR: {
        'type': 'string',
        'maxlength': MAX_LOC_STRING
    },
    latitude_STR: {
        'type': 'float',
        'coerce': float
    },
    longitude_STR: {
        'type': 'float',
        'coerce': float
    },
    speed_STR: {
        'type': 'float',
        'coerce': float
    },
    altitude_STR: {
        'type': 'float',
        'coerce': float
    },
    height_STR: {
        'type': 'float',
        'coerce': float
    },
    lat_err_STR: {
        'type': 'float',
        'coerce': float
    },
    long_err_STR: {
        'type': 'float',
        'coerce': float
    },
    alt_err_STR: {
        'type': 'float',
        'coerce': float
    },
    gps_hours_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_minutes_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_seconds_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_microseconds_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_day_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_month_STR:  {
        'type': 'integer',
        'coerce': int
    },
    gps_year_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_fix_quality_STR: {
        'type': 'float',
        'coerce': float
    },
    gps_satellites_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_total_sats_STR: {
        'type': 'integer',
        'coerce': int
    },
    gps_gdata_good_STR: {
        'type': 'integer',
        'coerce': int
    }
}  # DONE


# numbers that indicate true or false
GOOD_NUM_TRUE = 1
BAD_NUM_FALSE = 0

# time string
TIME_STR_FMT = '%d/%m/%Y %H:%M:%S'

# IP lookup database directory
DB_IP_LOOKUP_WIN = 'c:/maxdb/GeoIP/'
DB_IP_LOOKUP_LINUX = '/usr/share/GeoIP/'    # for https://github.com/maxmind/geoipupdate
DB_IP_LOOKUP_DOCKER = DB_IP_LOOKUP_LINUX    # same directory used for Docker
DB_IP_CHECK_COUNTRY = 'GeoLite2-Country.mmdb'
DB_IP_CHECK_CITY = 'GeoLite2-City.mmdb'
DB_IP_CHECK_ASN = 'GeoLite2-ASN.mmdb'

# Allowed locations
LOCALHOST_LIST = ['localhost', '127.0.0.1']
ALLOWED_GEO_LOC = ['Canada']
COUNTRY_STR = 'country'
CITY_STR = 'city'
ASN_STR = 'asn'
IP_ADDR_STR = 'addr'

# dictionary time identifer for logging the observation
LOG_TIME_STR = 'time'
TABLE_STR = 'measurement'
IP_DATABASE = 'ip_log'
MEASUREMENT_TABLE_IP = 'ip'
SENSOR_DATABASE = 'sensor_data'
DATA_TABLE_IP = 'data'
FIELDS_STR = 'fields'

# range of days over which to accept the data
TIME_DAY = 5

# names of variables to use in the endpoints
STARTING_TIME_WEB = '6 October 2020 9:00 AM'
# STARTING_TIME_WEB = 'A week ago'
ENDING_TIME_WEB = None
TURBIDITY_VAR = a0_voltage_STR
TDS_VAR = a1_voltage_STR
TEMP_VAR = temp0_STR

# number of decimal places to use for GPS mapping
DEC_PLACE_GPS = 8
# number of decimal places to use for TDS/Turbidity voltages
VOLTAGE_DEC_PLACES = 4
# temperature decimal places to use
TEMP_DEC_PLACES = 2
# temperature low
TEMP_LOW = -40


