#! /usr/bin/env python2

from bs4 import BeautifulSoup
import urllib2
import re

def get_links_from_piratebay():
    ret = urllib2.urlopen('https://thepiratebay.se/top/200')

    soup = BeautifulSoup(ret.read(), "lxml")
    magnet_hrefs = soup.find_all('a', attrs={'href': re.compile('^magnet')})
    return map(lambda x: x['href'], magnet_hrefs)

if __name__ == '__main__':
    links = get_links_from_piratebay()
    print '\n'.join(links)


