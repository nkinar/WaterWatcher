import json
import pymongo
from loguru import logger
from cerberus import Validator
import datetime
import json
from Influx import Influx
from constants import *
from secrets import *
from datetime import datetime, timedelta
from loguru import logger


class DoAllProcessing:

    def __init__(self):
        self.client = pymongo.MongoClient("mongodb://localhost:{0}/".format(MONGO_PORT))
        self.db = self.client[WW_SENSOR_DB_NAME]
        self.col = self.db[WW_SENSOR_COL_REG]
        self.influx = Influx()

    def validate_data_initial(self, doc):
        v = Validator(DATA_SCHEMA)
        if not v.validate(doc):
            logger.debug('Validation errors: ' + json.dumps(v.errors))
            raise ValueError
        sensor_num = doc[SENSOR_NUM_FIELD]
        sensor_token = doc[SENSOR_TOKEN_FIELD]
        record = {  # ensure that we have a dict with only two keys
            SENSOR_NUM_FIELD: sensor_num,
            SENSOR_TOKEN_FIELD: sensor_token
        }
        if not self.col.find_one(record):
            raise ValueError

    def process_json(self, d):
        v = Validator(DATA_SCHEMA)
        if not v.validate(d):
            raise ValueError
        self.add_to_database(d)

    def do_all_processing(self, input_str):
        try:
            doc = json.loads(input_str)
            self.validate_data_initial(doc)
            self.process_json(doc)
        except ValueError as e:
            print('Exception: ' + str(e))
            return False
        return True  # everything has succeeded

    def isNowInTimePeriod(self, startTime, endTime, nowTime):
        """
        https://stackoverflow.com/questions/10048249/how-do-i-determine-if-current-time-is-within-a-specified-range-using-pythons-da
        :param startTime:
        :param endTime:
        :param nowTime:
        :return:
        """
        if startTime < endTime:
            return nowTime >= startTime and nowTime <= endTime
        else:  # Over midnight
            return nowTime >= startTime or nowTime <= endTime

    def add_to_database(self, d):
        """
        This function adds all of the data to the database
        :param d:
        :return:
        """
        # take the starting time as the time of measurement
        start_time = datetime.strptime(d[start_time_str_STR], TIME_STR_FMT)
        logger.debug('Start time:' + str(start_time))
        dt = timedelta(days=TIME_DAY)
        if not self.isNowInTimePeriod(start_time - dt, start_time + dt, start_time):
            start_time = datetime.now()  # take the timestamp as now if the time is not right
            logger.debug('Setting start time to now since time is not valid: ' + str(start_time))
        rv = self.influx.insert(d, SENSOR_DATABASE, DATA_TABLE_IP, time=str(start_time))
        logger.debug('Done adding to database:' + str(rv))


