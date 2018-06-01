#! /bin/bash
# use this script for downloading Golestan CAPTCHA
for ((i=0;i < 10000000;i++)){
    wget -x --no-check-certificate https://support.nowpardaz.ir/frm/captcha/captcha.ashx -O ./$i.gif
}
exit 0