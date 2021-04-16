from flask import Flask, jsonify
from DataEndPointProcessor import DataEndPointProcessor
from constants import *

app = Flask(__name__)
ep = DataEndPointProcessor()


@app.route('/api/turbidity')
def get_turbidity():
    out = ep.get_turbidity()
    return jsonify(out)


@app.route('/api/tds')
def get_tds():
    out = ep.get_tds()
    return jsonify(out)


@app.route('/api/temp')
def get_temp():
    out = ep.get_temp()
    return jsonify(out)


@app.route('/api/loc')
def get_loc():
    out = ep.get_loc()
    return jsonify(out)


if __name__ == '__main_':
    app.run(debug=True, port=WEB_GUI_PORT)

