import urllib
import urllib2
import cookielib
import time
import os
import re
import os.path
import shutil
import sys

from pytesser import *

cj=cookielib.CookieJar()

FALSE = 0
TRUE  = 1

filename = 'd:/guet-file.html'

def login(username,password):
    opener = urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    urllib2.install_opener(opener)
    opener.addheaders.append(('User-Agent', 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1'))
    y = opener.open('http://onlinejudge.guet.edu.cn/guetoj/site/captcha.html')
    f = open('verify.png', "wb")
    f.write(y.read())
    f.close()
    image = Image.open('verify.png')
    verifyCode = image_to_string(image).rstrip()
    print 'Login:'+username,password,verifyCode
    url = 'http://onlinejudge.guet.edu.cn/guetoj/site/login.html'
    login_data = {'LoginForm[username]':username,'LoginForm[password]':password, 'LoginForm[rememberMe]':'1', 'LoginForm[verifyCode]':verifyCode, 'yt0':'Login'}
    data = urllib.urlencode(login_data)
    print data
    request=urllib2.Request(url, data)
    urlcontent=opener.open(request)
    print 'goto: ' + urlcontent.geturl()
    if (cmp("http://onlinejudge.guet.edu.cn/guetoj/",urlcontent.geturl())==0):
        print 'Login ok...'
        return TRUE
    else:
        print 'Login failed...'
        return FALSE

#status
def status(username):
    print 'start to get status...'

    #<tr class="odd">
    #<td>1476</td><td class="link-column"><a href="/guetoj/problem/view/1000.html">1000</a></td>
    #<td class="link-column"><a href="/guetoj/user/view/1000847.html">vjudge</a></td>
    #<td>G++</td><td>9</td><td>800</td><td>2013-12-27 22:58:42</td><td>Accepted</td></tr>
    #<tr class="even">

    url='http://onlinejudge.guet.edu.cn/guetoj/user/submissions/'+username+'.html'
    data = urllib.urlopen(url)
    html_content =data.read()

    f = open(filename,'w')
    f.write(html_content)
    f.close()

    print 'save status_content to ',filename

def submit(pid,lang,source_path):
    source = ''
    f=file(source_path)
    # if no mode is specified, 'r'ead mode is assumed by default
    while True:
        line = f.readline()
        if len(line) == 0: # Zero length indicates EOF
            break
        #print line,
        source += line
        # Notice comma to avoid automatic newline added by Python
    f.close()
    # close the file

    print source

    #incase that less than 50 byte
    source += str('                                                 ');

    url='http://onlinejudge.guet.edu.cn/guetoj/problem/submit/'+pid+'.html'
    values={'Problem[source_code]':source,'Problem[language]':lang}
    data = urllib.urlencode(values)
    headers ={"User-agent":"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1"}
    req = urllib2.Request(url, data,headers)
    opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(cj))
    data = urllib.urlencode(values)
    request=urllib2.Request(url, data)
    urlcontent=opener.open(request)
    print 'goto: ' + urlcontent.geturl()

    if (0==cmp('http://onlinejudge.guet.edu.cn/guetoj/runs.html',urlcontent.geturl())):
        print 'Submit ok...'
        return TRUE
    else:
        print 'Submit failed...'
        return FALSE

def compileError(runId):
    #onlinejudge.guet.edu.cn/guetoj/run/view/1474.html
    url='onlinejudge.guet.edu.cn/guetoj/run/view/'+runId+'.html'
    data = urllib.urlopen(url)
    html_content =data.read()

    f = open(filename,'w')
    f.write(html_content)
    f.close()

    print 'save compile_content to ',filename

if __name__=="__main__":

    while 1:
        count = len(sys.argv)
        print 'Argv-num :',count

        if count <= 1:
            print 'No argv, so break.'
            break

        print type(sys.argv)
        print str(sys.argv)

        #for a in range(1, len(sys.argv)):
        #    print sys.argv[a]

        if sys.argv[1] == 'submit':
            print 'Do submit...'
            pid = sys.argv[2]
            lang = sys.argv[3]
            username = sys.argv[4]
            password = sys.argv[5]
            source_path = sys.argv[6]
            filename = sys.argv[7]
            print pid,lang,username,password,source_path,filename

            x = urllib2.urlopen("http://onlinejudge.guet.edu.cn/guetoj/site/captcha.html?refresh=1")
            loop = 20
            ret = FALSE
            while(ret!=TRUE and loop>0):
                ret = login(username, password)
                loop=loop-1;
            loop = 20
            ret = FALSE
            while(ret!=TRUE and loop>0):
                ret = submit(pid,lang,source_path)
                loop=loop-1;
            break

        if sys.argv[1] == 'status':
            print 'Do status...'
            username = sys.argv[2]
            filename = sys.argv[3]
            status(username)
            break

        if sys.argv[1] == 'ce':
            print 'Do ce...'
            rid = sys.argv[2]
            filename = sys.argv[3]
            print rid,filename
            compileError(rid)
            break
        else:
            print "Error: Command line option syntax error, the follow fomart supported."
            print " 1: submit pid lang username password source-path return-file-path"
            print " 2: status username return-file-path"
            print " 3: ce rid return-file-path"
            break


