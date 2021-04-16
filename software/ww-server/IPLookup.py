from constants import *
from is_docker import is_docker
import platform
import geoip2.database


class IPLookup:
    def __init__(self):
        self.rc = geoip2.database.Reader(self.get_db_path(DB_IP_CHECK_COUNTRY))
        self.rcity = geoip2.database.Reader(self.get_db_path(DB_IP_CHECK_CITY))
        self.asn = geoip2.database.Reader(self.get_db_path(DB_IP_CHECK_ASN))

    def get_db_path(self, name):
        stem = ''
        if is_docker():
            stem = DB_IP_LOOKUP_DOCKER
        elif platform.system() == 'Linux':
            stem = DB_IP_LOOKUP_LINUX
        elif platform.system() == 'Windows':
            stem = DB_IP_LOOKUP_WIN
        else:
            stem = DB_IP_LOOKUP_LINUX  # default to Linux
        return stem + name

    def lookup_country(self, ip):
        response = self.rc.country(ip)
        return response.country.name

    def determine_allowed(self, ip):
        if ALLOW_LOCALHOST and ip in LOCALHOST_LIST:
            return True
        if self.lookup_country(ip) in ALLOWED_GEO_LOC:
            return True
        return False

    def get_info_ip(self, ip):
        d = {
            COUNTRY_STR: '',
            CITY_STR: '',
            ASN_STR: '',
            IP_ADDR_STR: ip
        }
        if ALLOW_LOCALHOST and ip in LOCALHOST_LIST:
            return d
        print('in response lookup')
        response_city = self.rcity.city(ip)
        response_asn = self.asn.asn(ip)
        country_name = response_city.country.name
        city_name = response_city.city.name
        asn = response_asn.autonomous_system_organization
        d[COUNTRY_STR] = country_name
        d[CITY_STR] = city_name
        d[ASN_STR] = asn
        d[IP_ADDR_STR] = ip
        print('past repsonse lookup')
        return d


def main():
    pass


if __name__ == '__main__':
    main()
