import time
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
import random
import os
import platform
import psutil
from constants import *


class ChromeDownloader:
    def __init__(self):
        """
        Initialize the class
        """
        self.download_dir = None
        self.filelink_name = None
        self.max_wait_time = MAX_DELAY_TIME
        self.min_wait_time = MIN_DELAY_TIME
        self.chrome_user_dir = self.get_chrome_user_dir()
        self.links = []

    def set_download_dir(self, directory):
        """
        Set the download directory
        :param dir:         as the directory name
        :return:
        """
        self.download_dir = self.get_full_dir_name(directory)

    def set_filelink_name(self, fl_name):
        """
        Set the name of the file with the links
        :param name:        of the file
        :return:
        """
        self.filelink_name = self.get_full_dir_name(fl_name)

    def set_max_wait_time(self, t):
        """
        Set the max wait time between successive file downloads
        :param t:       as the time in seconds
        :return:
        """
        self.max_wait_time = t

    def set_min_wait_time(self, t):
        """
        Set the min wait time between successive file downloads
        :param t:       as the time in seconds
        :return:
        """
        self.min_wait_time = t

    def get_random_time(self):
        """
        Obtain a random delay time
        :return:
        """
        return int(random.randint(self.min_wait_time, self.max_wait_time))

    def download_files_work(self, pn):
        """
        Function to download files from links
        :param pn:      as the list of links to download
        :return:
        """
        chrome_options = Options()
        chrome_options.add_argument('--headless')   # do not show the browser
        chrome_options.add_argument('log-level=3')  # show only fatal errors to not clutter the screen
        chrome_options.add_argument("user-data-dir=" + self.chrome_user_dir)  # run browser as current user
        chrome_options.add_experimental_option("prefs", {"download.default_directory": self.download_dir,
                                                         "download.prompt_for_download": False,
                                                         "download.directory_upgrade": True,
                                                         "safebrowsing.enabled": True})
        driver = webdriver.Chrome(options=chrome_options)
        n = len(pn)
        for k in range(n):
            print('{0}/{1} files'.format(k+1, n))
            print('link: {0}'.format(pn[k]))
            driver.get(pn[k])
            time.sleep(self.get_random_time())
        driver.quit()  # quit the web browser
        # DONE

    def load_link_list(self):
        """
        Load in the links from a file
        :return:
        """
        links = []
        with open(self.filelink_name) as fp:
            lines = fp.readlines()
        for line in lines:
            line = line.strip()
            if not line.startswith(COMMENT_CHAR) and line:
                links.append(line)
        return links

    def download_files(self):
        """
        Load the files and download the data
        :return:
        """
        if self.detect_chrome_running():
            raise Exception('Chrome is already running - please close the browser and re-run this script')
        if self.download_dir is None:
           raise Exception('Download directory needs to be set before continuing.')
        if self.filelink_name is None:
            raise Exception('Input filename needs to be set before continuing')
        links = self.load_link_list()
        self.download_files_work(links)

    def get_chrome_user_dir(self):
        """
        Obtain the Chrome user directory
        :return:    The name of the chrome user directory on the OS
        """
        check_platform = platform.system()
        p = None
        if check_platform is WINDOWS_STR:   # Windows
            p = os.environ['USERPROFILE'] + CHROME_APP_DATA_WIN
        elif check_platform is DARWIN_STR:  # Mac OS X
            p = os.getenv("HOME") + CHROME_APP_DATA_MAC
        else:  # other platforms
            p = os.getenv("HOME") + CHROME_APP_DATA_LINUX
        return p

    def detect_chrome_running(self):
        """
        Check if Chrome is running. If Chrome is running, then return True.
        :return:
        True if Chrome is running
        False if Chrome is not running
        """
        check_platform = platform.system()
        pname = None
        if check_platform is WINDOWS_STR:   # Windows
            pname = CHROME_EXE_WIN
        elif check_platform is DARWIN_STR:  # Mac OS X
            pname = CHROME_EXE_MAC
        else:
            pname = CHROME_EXE_LINUX        # Linux
        for p in psutil.process_iter():
            if p.name() == pname:
                return True
        return False

    def get_full_dir_name(self, name):
        """
        Obtain the full output name of the directory or file
        :param name:       as the local or relative path of the directory or file
        :return:
        """
        if os.path.exists(name):
            return os.path.abspath(name)
        raise Exception('Input file or directory: {0} does not exist'.format(name))


########################################################################


def main():
    cd = ChromeDownloader()
    cd.set_download_dir('./downloaded')
    cd.set_filelink_name('files-download.txt')
    cd.download_files()


if __name__ == '__main__':
    main()

