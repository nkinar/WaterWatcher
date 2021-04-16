import socket
import click
import json
import strgen
from IPy import IP
from datetime import datetime, timedelta
import random
from constants import *


def gen_serial_number():
    return strgen.StringGenerator(TOKEN_STR_SERIAL).render()


def obtain_current_time(add_sec=None):
    obj = datetime.now()
    if add_sec:
        obj += timedelta(0, add_sec)
    return obj.strftime(TIME_STR_FMT)


def gen_json(num, token):
    dt = datetime.today()
    d = {
        SENSOR_NUM_FIELD: num,
        SENSOR_TOKEN_FIELD: token,

        # scientific variables
        a0_voltage_STR: random.uniform(0, 5),
        a0_out_STR: random.uniform(0, 5),
        a1_voltage_STR: random.uniform(0, 5),
        a1_out_STR: random.uniform(0, 5),
        a2_voltage_STR: random.uniform(0, 5),
        a2_out_STR: random.uniform(0, 5),
        temp0_STR: random.uniform(2, 23),
        temp0_out_STR: random.uniform(2, 23),

        # system health variables
        serial_number_STR: gen_serial_number(),
        serial_number_good_STR: GOOD_NUM_TRUE,
        battery_fault_STR: False,
        battery_charging_STR: False,
        rtc_temperature_STR: random.uniform(2, 23),
        start_time_str_STR: obtain_current_time(),
        end_time_str_STR: obtain_current_time(3),
        btemperature_STR: 25.2,
        bvoltage_STR: 8.5,
        bcurrent_STR: 0.040,
        bcapacity_STR: 0.34,
        uptime_STR: 720,
        name_STR: "POND",
        latitude_STR: 52.1332,
        longitude_STR: -106.6700,
        speed_STR: 1.08,
        altitude_STR: 482,
        height_STR: 498,
        lat_err_STR: 0.0033,
        long_err_STR: 0.00234,
        alt_err_STR: 0.003,
        gps_hours_STR: dt.hour,
        gps_minutes_STR: dt.minute,
        gps_seconds_STR: dt.second,
        gps_microseconds_STR: dt.microsecond,
        gps_day_STR: dt.day,
        gps_month_STR: dt.month,
        gps_year_STR: dt.year,
        gps_fix_quality_STR: 0.02,
        gps_satellites_STR: 7,
        gps_total_sats_STR: 12,
        gps_gdata_good_STR: 1
    }  # DONE
    return d


def wait_recev(soc):
    buffer = ""
    try:
        while True:
            client_input = soc.recv(MAX_CHAR_LEN)
            if not client_input:  # empty string or connection
                break
            buffer += client_input.decode()
            if len(buffer) > MAX_CHAR_LEN:
                break
            if buffer.endswith(CR_STR):
                print(buffer)  # print the response
                break
    except socket.timeout as e:
        print('Timeout occurred: ' + str(e))


def send(sdata, src_ip=None):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as soc:
        soc.settimeout(SOCKET_TIMEOUT)
        if src_ip:
            try:
                IP(src_ip)
            except ValueError as e:
                print('Source IP address is not valid, exiting...')
                return
            soc.bind((src_ip, 0))  # spoof the source IP
        # soc.connect((HOST, SERVPORT))
        # soc.connect(('3.tcp.ngrok.io', 22684))
        # soc.connect(('52.12.226.56', 8000))
        soc.sendall(str.encode(sdata + CR_STR))  # string needs to be terminated with \r
        wait_recev(soc)


@click.command()
@click.option('-n', '--number', required=True, help='Sensor number', type=int)
@click.option('-t', '--token', required=True, help='Token used for connection', type=str)
@click.option('-p', '--print', required=False, help='Just print the output and do not send', is_flag=True)
@click.option('-sip', '--src-ip', required=False, help='Set source IP addr', type=str)
def start(number, token, print, src_ip):
    data = json.dumps(gen_json(number, token), ensure_ascii=True)
    click.echo('DATA SENT:')
    click.echo(data)
    if not print:   # only send if we are not printing
        send(data, src_ip)
    click.echo("DONE")


if __name__ == "__main__":
    start()


