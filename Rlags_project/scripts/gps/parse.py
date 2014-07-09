#!/usr/bin/python
# URL that generated this code:
# http://txt2re.com/index-python.php3?s=$GPRMC,172756.000,A,3146.7581,N,09542.9878,W,0.1,0.0,060314,0.0,W*66&80&74&82&89&-116&18&165&65&100&87&83&79&-117&19&168

import re
import sys

# txt='$GPRMC,172756.000,A,3146.7581,N,09542.9878,W,0.1,0.0,060314,0.0,W*66'

while 1:
  txt = raw_input()

  re1='.*?' # Non-greedy match on filler
  re2='\\d' # Uninteresting: d
  re3='.*?' # Non-greedy match on filler
  re4='\\d' # Uninteresting: d
  re5='.*?' # Non-greedy match on filler
  re6='\\d' # Uninteresting: d
  re7='.*?' # Non-greedy match on filler
  re8='\\d' # Uninteresting: d
  re9='.*?' # Non-greedy match on filler
  re10='\\d'  # Uninteresting: d
  re11='.*?'  # Non-greedy match on filler
  re12='\\d'  # Uninteresting: d
  re13='.*?'  # Non-greedy match on filler
  re14='\\d'  # Uninteresting: d
  re15='.*?'  # Non-greedy match on filler
  re16='\\d'  # Uninteresting: d
  re17='.*?'  # Non-greedy match on filler
  re18='\\d'  # Uninteresting: d
  re19='.*?'  # Non-greedy match on filler
  re20='(\\d)'  # Any Single Digit 1
  re21='(\\d)'  # Any Single Digit 2
  re22='(\\d)'  # Any Single Digit 3
  re23='(\\d)'  # Any Single Digit 4
  re24='(\\.)'  # Any Single Character 1
  re25='(\\d+)' # Integer Number 1
  re26='.*?'  # Non-greedy match on filler
  re27='.'  # Uninteresting: c
  re28='.*?'  # Non-greedy match on filler
  re29='(.)'  # Any Single Character 2
  re30='.*?'  # Non-greedy match on filler
  re31='(\\d)'  # Any Single Digit 5
  re32='(\\d)'  # Any Single Digit 6
  re33='(\\d)'  # Any Single Digit 7
  re34='(\\d)'  # Any Single Digit 8
  re35='(\\d)'  # Any Single Digit 9
  re36='(\\.)'  # Any Single Character 3
  re37='(\\d+)' # Integer Number 2
  re38='.*?'  # Non-greedy match on filler
  re39='.'  # Uninteresting: c
  re40='.*?'  # Non-greedy match on filler
  re41='(.)'  # Any Single Character 4

  rg = re.compile(re1+re2+re3+re4+re5+re6+re7+re8+re9+re10+re11+re12+re13+re14+re15+re16+re17+re18+re19+re20+re21+re22+re23+re24+re25+re26+re27+re28+re29+re30+re31+re32+re33+re34+re35+re36+re37+re38+re39+re40+re41,re.IGNORECASE|re.DOTALL)
  m = rg.search(txt)
  if m:
      d1=m.group(1)
      d2=m.group(2)
      d3=m.group(3)
      d4=m.group(4)
      c1=m.group(5)
      int1=m.group(6)
      c2=m.group(7)
      d5=m.group(8)
      d6=m.group(9)
      d7=m.group(10)
      d8=m.group(11)
      d9=m.group(12)
      c3=m.group(13)
      int2=m.group(14)
      c4=m.group(15)
      if c2 == 'N':
        if c4 == 'E':
          print d1+d2+","+d3+d4+c1+int1+","+d5+d6+d7+","+d8+d9+c3+int2
        else:
          print d1+d2+","+d3+d4+c1+int1+",-"+d5+d6+d7+",-"+d8+d9+c3+int2
      else:
        if c4 == 'E':
          print "-"+d1+d2+",-"+d3+d4+c1+int1+","+d5+d6+d7+","+d8+d9+c3+int2
        else:
          print "-"+d1+d2+",-"+d3+d4+c1+int1+",-"+d5+d6+d7+",-"+d8+d9+c3+int2
