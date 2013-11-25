#from http_parser import HTTPParser
from http_parser import HTTPParser


data = "GET / HTTP/1.0\r\nUser-Agent: Wget/1.11.4\r\nAccept: */*\r\nHost: www.163.com\r\nConnection: Keep-Alive\r\n\r\n"
env = dict()
parser =  HTTPParser()
#parser.execute(env, data, 0)
parser.execute(env, data, 0)
print env


