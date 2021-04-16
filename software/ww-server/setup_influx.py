from influxdb import InfluxDBClient
from constants import *


def setup_databases():
    print('Setting up the Influx databases...')
    client = InfluxDBClient(host=INFLUX_HOST, port=INFLUX_PORT)
    client.create_database(IP_DATABASE)
    client.create_database(SENSOR_DATABASE)
    print('DONE')


def main():
    setup_databases()


if __name__ == "__main__":
    main()



