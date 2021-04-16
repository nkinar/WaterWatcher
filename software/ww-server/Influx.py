import datetime
from influxdb import InfluxDBClient, exceptions
from loguru import logger
import dateparser
import datetime
from constants import *

"""
Example to query the Influx database:
    f = Influx()
    database = 'sensor_data'
    table = 'data'
    t1 = 'A week ago'
    t2 = 'tomorrow'
    name = 'a0_out'
    print(f.obtain_data(database, table, name, t1, t2))
    print(f.obtain_data_as_list_of_lists(database, table, name, t1, t2))
"""


class Influx:
    def __init__(self):
        self.client = InfluxDBClient(host=INFLUX_HOST, port=INFLUX_PORT)

    def insert(self, d, database, table, time=None):
        """
        Insert into InfluxDB
        :param d:               as the fields to be stored
        :param database:        as the name of the database
        :param table:           as the name of the table
        :param time:            as the time in canonical format
        :return:                True if the data has been stored
        """
        try:
            self.client.switch_database(database)
            if time is None:  # obtain the current timestamp
                time = str(datetime.datetime.now())
            dout = dict()
            dout[LOG_TIME_STR] = time
            dout[TABLE_STR] = table
            dout[FIELDS_STR] = d
            logger.debug('Influx in:' + str(dout))
            resp = self.client.write_points([dout])
            self.client.close()
            return resp
        except exceptions.InfluxDBClientError as e:
            raise ValueError(str(e))

    def obtain_data(self, database, table, name=None, t1=None, t2=None):
        """
        Obtain data between time ranges
        :param database:        as the database to query
        :param table:           as the table in the database
        :param name:            as the field name (None for all points)
        :param t1:              as the starting time (None for all time)
        :param t2:              as the ending time (None for all time)
        :return:
        This function will return the entire series in the database if t1==t2==None
        Otherwise, the function will return the time series between the two times t1 and t2.
        The output is an ordered list with each element in the list as a dictionary containing the points.
        """
        t1_txt = ''
        t2_txt = ''
        s = '*'
        if name is not None:
            s = name
        self.client.switch_database(database)
        q = 'select {0} from "{1}"'.format(s, table)
        if t1 is not None:
            t1_txt = "'" + str(dateparser.parse(t1)) + "'"
            q += ' where time > ' + t1_txt + ' '
        if t2 is not None:
            t2_txt = "'" + str(dateparser.parse(t2)) + "'"
            if t1 is not None:
                q += 'and '
            else:
                q += ' where '
            q += 'time < ' + t2_txt + ';'
        out = self.client.query(q)
        return list(out.get_points())


    def obtain_data_as_list_of_lists(self, database, table, name, t1=None, t2=None, dec_places=2, remove_less_than=None):
        """
        Function to obtain data as a list of lists for output to Flask for web plotting
        [[time0, data0], [time1, data1],...]
        :param database:            as the database name
        :param table:               as the table
        :param name:                as the name of the measurement taken from the database (Required)
        :param t1:                  starting time (None for all)
        :param t2:                  ending time (None for all)
        :param dec_places:          Default number of decimal places to export
        :param remove_less_than:    Lowest number to remove from the list
        :return:
        A list of lists with the data
        """
        r = self.obtain_data(database, table, name, t1, t2)
        out = []
        n = len(r)
        for k in range(n):
            d = r[k]
            time = d[LOG_TIME_STR]
            m = round(d[name], dec_places)
            if remove_less_than is not None:
                if m >= remove_less_than:
                    out.append([time, m])
            else:  # remove_less_than is None
                out.append([time, m])
        return out


def main():
    pass


if __name__ == '__main__':
    main()


