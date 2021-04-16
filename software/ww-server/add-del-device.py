import pymongo
import strgen
import click
from constants import *

"""
Script to add a device token to the database
"""

def gen_token():
    return strgen.StringGenerator(TOKEN_STR).render()


def add_del_device(d, op):
    try:
        client = pymongo.MongoClient("mongodb://localhost:{0}/".format(MONGO_PORT))
        db = client[WW_SENSOR_DB_NAME]
        col = db[WW_SENSOR_COL_REG]
        if op == OP_ADD:
            if col.find_one({SENSOR_NUM_FIELD: d[SENSOR_NUM_FIELD]}):
                print('Cannot add doc that exists, use del function first')
                return
            if d[SENSOR_TOKEN_FIELD] is None:
                token = gen_token()
                print('Token is not provided, generating token: ', token)
                d[SENSOR_TOKEN_FIELD] = token
            x = col.insert_one(d)
            print('Added Record ID: ' + str(x.inserted_id))
        elif op == OP_DEL:
            if d[SENSOR_TOKEN_FIELD] is None:
                del d[SENSOR_TOKEN_FIELD]
            x = col.delete_one(d)
            print('Deleted Number of Records: ' + str(x.deleted_count))
        elif op == OP_INFO:
            print('Records:')
            for x in col.find({}):
                print(x)
        else:
            print('Operation command is not implemented')
            return
    except Exception as e:
        print('Exception: ', e)


@click.command()
@click.option('-op', '--operation', required=True, help='Specify operation [add | del | info]', type=str)
@click.option('-n', '--number', required=False, help='Sensor number', type=int)
@click.option('-t', '--token', required=False, help='Token used for connection', type=str)
def start(operation, number, token):
    if token is not None:
        if len(token) > TOKEN_CHARS:
            click.Abort('The token length must be <= {0} chars'.format(TOKEN_CHARS))
    if operation != OP_DEL:
        if number is None:
            click.Abort('Sensor number must be provided.')
    d = {
        SENSOR_NUM_FIELD: number,
        SENSOR_TOKEN_FIELD: token
    }
    try:
        add_del_device(d, operation)
    except Exception as e:
        click.Abort('Exception: ' + str(e))


if __name__ == "__main__":
    start()

