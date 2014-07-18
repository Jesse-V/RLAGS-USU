#!/usr/bin/python
# URL that generated this code:
# http://txt2re.com/index-python.php3?s=0.010927,0.011606,0.999297,-0.140806,-0.047336,-0.031671,0.006311,-0.006656,-0.426104,0.921638,-0.388030,-0.003924,-0.388026,-0.921647,0.001928,-0.004365,-0.000255,-0.999990,0.224831,-0.110486,-22.832188,46.523895,1.169785&36&6&14&7&4&31&13&15&3

import re
import sys

txt = raw_input()
# txt='0.010927,0.011606,0.999297,-0.140806,-0.047336,-0.031671,0.006311,-0.006656,-0.426104,0.921638,-0.388030,-0.003924,-0.388026,-0.921647,0.001928,-0.004365,-0.000255,-0.999990,0.224831,-0.110486,-22.832188,46.523895,1.169785'

re1='.*?' # Non-greedy match on filler
re2='[+-]?\\d*\\.\\d+(?![-+0-9\\.])'  # Uninteresting: float
re3='.*?' # Non-greedy match on filler
re4='[+-]?\\d*\\.\\d+(?![-+0-9\\.])'  # Uninteresting: float
re5='.*?' # Non-greedy match on filler
re6='[+-]?\\d*\\.\\d+(?![-+0-9\\.])'  # Uninteresting: float
re7='.*?' # Non-greedy match on filler
re8='[+-]?\\d*\\.\\d+(?![-+0-9\\.])'  # Uninteresting: float
re9='.*?' # Non-greedy match on filler
re10='[+-]?\\d*\\.\\d+(?![-+0-9\\.])' # Uninteresting: float
re11='.*?'  # Non-greedy match on filler
re12='[+-]?\\d*\\.\\d+(?![-+0-9\\.])' # Uninteresting: float
re13='.*?'  # Non-greedy match on filler
re14='[+-]?\\d*\\.\\d+(?![-+0-9\\.])' # Uninteresting: float
re15='.*?'  # Non-greedy match on filler
re16='[+-]?\\d*\\.\\d+(?![-+0-9\\.])' # Uninteresting: float
re17='.*?'  # Non-greedy match on filler
re18='[+-]?\\d*\\.\\d+(?![-+0-9\\.])' # Uninteresting: float
re19='.*?'  # Non-greedy match on filler
re20='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 1
re21='.*?'  # Non-greedy match on filler
re22='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 2
re23='.*?'  # Non-greedy match on filler
re24='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 3
re25='.*?'  # Non-greedy match on filler
re26='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 4
re27='.*?'  # Non-greedy match on filler
re28='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 5
re29='.*?'  # Non-greedy match on filler
re30='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 6
re31='.*?'  # Non-greedy match on filler
re32='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 7
re33='.*?'  # Non-greedy match on filler
re34='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 8
re35='.*?'  # Non-greedy match on filler
re36='([+-]?\\d*\\.\\d+)(?![-+0-9\\.])' # Float 9

rg = re.compile(re1+re2+re3+re4+re5+re6+re7+re8+re9+re10+re11+re12+re13+re14+re15+re16+re17+re18+re19+re20+re21+re22+re23+re24+re25+re26+re27+re28+re29+re30+re31+re32+re33+re34+re35+re36,re.IGNORECASE|re.DOTALL)
m = rg.search(txt)
if m:
    float1=m.group(1)
    float2=m.group(2)
    float3=m.group(3)
    float4=m.group(4)
    float5=m.group(5)
    float6=m.group(6)
    float7=m.group(7)
    float8=m.group(8)
    float9=m.group(9)
    print float1+" "+float2+" "+float3+" "+float4+" "+float5+" "+float6+" "+float7+" "+float8+" "+float9+" "
