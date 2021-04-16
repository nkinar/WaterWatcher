from Influx import Influx
import math
from constants import *


class DataEndPointProcessor:
    def __init__(self):
        self.influx = Influx()

    def get_turbidity(self):
        name = TURBIDITY_VAR
        out = self.influx.obtain_data_as_list_of_lists(SENSOR_DATABASE, DATA_TABLE_IP, name, STARTING_TIME_WEB, ENDING_TIME_WEB, VOLTAGE_DEC_PLACES)
        # TODO: Apply a calibration coefficient to the voltage when required
        return out

    def get_tds(self):
        name = TDS_VAR
        out = self.influx.obtain_data_as_list_of_lists(SENSOR_DATABASE, DATA_TABLE_IP, name, STARTING_TIME_WEB, ENDING_TIME_WEB, VOLTAGE_DEC_PLACES)
        # TODO: Apply a calibration coefficient to the voltage when required
        return out

    def get_temp(self):
        name = TEMP_VAR
        out = self.influx.obtain_data_as_list_of_lists(SENSOR_DATABASE, DATA_TABLE_IP, name, STARTING_TIME_WEB, ENDING_TIME_WEB, TEMP_DEC_PLACES, TEMP_LOW)
        # TODO: Apply a calibration coefficient to the voltage when required
        return out

    def get_last_elem_not_nan(self, lst, mag=None):
        for elem in reversed(lst):
            if not math.isnan(elem[1]):
                if mag is None:
                    return elem[1]
                else:  # mag is not None
                    if abs(elem[1]) < mag:
                        return elem[1]
        return -999  # default return since no value could be found

    def get_loc(self):
        lat = self.influx.obtain_data_as_list_of_lists(SENSOR_DATABASE, DATA_TABLE_IP, latitude_STR,
                                                       STARTING_TIME_WEB, ENDING_TIME_WEB, DEC_PLACE_GPS)
        lng = self.influx.obtain_data_as_list_of_lists(SENSOR_DATABASE, DATA_TABLE_IP, longitude_STR,
                                                       STARTING_TIME_WEB, ENDING_TIME_WEB, DEC_PLACE_GPS)
        lat_out = self.get_last_elem_not_nan(lat, 90)   # lat range is between -90 and 90
        lng_out = self.get_last_elem_not_nan(lng, 180)  # long range is between -180 and 180
        return [lat_out, lng_out]


def main():
    pass


if __name__ == '__main__':
    main()

