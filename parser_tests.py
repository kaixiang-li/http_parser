import pprint
from http_parser import HTTPParser


pp = pprint.PrettyPrinter(indent=4)
data = "GET / HTTP/1.0\r\nUser-Agent: Wget/1.11.4\r\nAccept: */*\r\nHost: www.douban.com\r\nConnection: Keep-Alive\r\n\r\n"
env = {
    'wsgi.input': "",
}
parser =  HTTPParser()
parser.execute(env, data, 0)
pp.pprint(env)


