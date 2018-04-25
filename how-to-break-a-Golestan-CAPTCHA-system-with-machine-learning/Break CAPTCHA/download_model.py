#!/usr/bin/env python

from __future__ import print_function
import hashlib
import sys
import tarfile
if sys.version_info[0] < 3:
    from urllib2 import urlopen
else:
    from urllib.request import urlopen


class Model:
    MB = 1024*1024
    BUFSIZE = 10*MB

    def __init__(self, **kwargs):
        self.name = kwargs.pop('name')
        self.url = kwargs.pop('url', None)
        self.filename = kwargs.pop('filename')
        self.sha = kwargs.pop('sha', None)
        self.archive = kwargs.pop('archive', None)
        self.member = kwargs.pop('member', None)

    def __str__(self):
        return 'Model <{}>'.format(self.name)

    def printRequest(self, r):
        def getMB(r):
            d = dict(r.info())
            for c in ['content-length', 'Content-Length']:
                if c in d:
                    return int(d[c]) / self.MB
            return '<unknown>'
        print('  {} {} [{} Mb]'.format(r.getcode(), r.msg, getMB(r)))

    def verify(self):
        if not self.sha:
            return False
        print('  expect {}'.format(m.sha))
        sha = hashlib.sha1()
        with open(self.filename, 'rb') as f:
            while True:
                buf = f.read(self.BUFSIZE)
                if not buf:
                    break
                sha.update(buf)
        print('  actual {}'.format(sha.hexdigest()))
        return self.sha == sha.hexdigest()

    def get(self):
        try:
            if self.verify():
                print('  hash match - skipping')
                return
        except Exception as e:
            print('  catch {}'.format(e))

        if self.archive or self.member:
            assert(self.archive and self.member)
            print('  hash check failed - extracting')
            print('  get {}'.format(self.member))
            self.extract()
        else:
            assert(self.url)
            print('  hash check failed - downloading')
            print('  get {}'.format(self.url))
            self.download()

        print(' done')
        print(' file {}'.format(self.filename))
        self.verify()

    def download(self):
        r = urlopen(self.url)
        self.printRequest(r)
        self.save(r)

    def extract(self):
        with tarfile.open(self.archive) as f:
            assert self.member in f.getnames()
            self.save(f.extractfile(self.member))

    def save(self, r):
        with open(self.filename, 'wb') as f:
            print('  progress ', end='')
            sys.stdout.flush()
            while True:
                buf = r.read(self.BUFSIZE)
                if not buf:
                    break
                f.write(buf)
                print('>', end='')
                sys.stdout.flush()

models = [
    Model(
        name='Alexnet',
        url='http://dl.caffe.berkeleyvision.org/bvlc_alexnet.caffemodel',
        sha='9116a64c0fbe4459d18f4bb6b56d647b63920377',
        filename='bvlc_alexnet.caffemodel')
]

# Note: models will be downloaded to current working directory
#       expected working directory is opencv_extra/testdata/dnn
if __name__ == '__main__':
    for m in models:
        print(m)
        m.get()