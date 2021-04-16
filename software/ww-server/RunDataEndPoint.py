from gevent.pywsgi import WSGIServer
from DataEndPoint import app
from constants import *

http_server = WSGIServer(('', WEB_GUI_PORT), app)
http_server.serve_forever(WEB_GUI_PORT)

