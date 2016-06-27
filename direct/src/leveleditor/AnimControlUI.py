"""
   This is the GUI for the Animation Control Panel
"""
from direct.interval.IntervalGlobal import *
from direct.actor.Actor import *
from . import ObjectGlobals as OG

import os, wx
from wx.lib.embeddedimage import PyEmbeddedImage

#----------------------------------------------------------------------
FirstFrame = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACh0lE"
    "QVRIie2Uz08aQRTHd368CZzYGGNjPWkTlB7EK0QSewSSamL9R+upXmpko/LLskgqhrDGyqb4"
    "Y40CB3dnd6aHMYBIjSZe2vhOk/2+93nf93Z3UOvsTHvtwK9OfIP+f1CEEKUUITRWJYQQQl4A"
    "RQgBpQihVqvV6/UwflBCCGGM3XY6tm33H9JhGWPMOR9xFwhhnZzs7e21bPvL+rqu60KIe3eU"
    "3tzcVCqVQqHwYW5ubW0tCIIBlDFmmmbTsjLptBoEADjnjUajWCodHx/3er2JiQklqWEdxzGr"
    "1R8HB7/bbU3TotHoqFOMcfv8vFarZdJpxpjruvV6vVAs1ut1z/MopQBACAEAAGi32xXTLJfL"
    "l5eXGGMAUJ3Gj88Y45w3LSu/v9+0LNd1AYAxpmlaEASE0uvr659HR+Vy2XEcla9pmpRS0zQy"
    "tOsBFCEkhPi2tZXP533fD4VCqqY/iue6Xzc3r66uGGPDUr98kNw/SSkJIclE4tPKiq7rnHNl"
    "QYUQIhwOp5aXF+bnlfERKKUDf3RYkFJOTk5ms9n40lJuZ6d6eOh5HgDcu5AyFoslk8liqWQY"
    "xsXFxcjnOQYqhPB9PwgCzvn76emNjY3FeNzI5ZrNpjKMCQmCAACSicR8NLqfz5dLpU63q9Dk"
    "sVMhRCQSmZmZQQhJKTnnCKGPsdjc7KxpmoZhtGxbCIEwVr11Xc9mMvHFxZ1crlarqfzBfp++"
    "+THGlNLbTmd3d/fX6enn1dV3U1O+7ysVAIQQjUbj+/Z2bGEhlUqpXT+AKpuP0WpxjuOEw+FQ"
    "KDScgxACgG63e3d3F4lEnuV0uFi1HNtVXQjq99VG3v4T8Tecij7uvsczoS+KN+g/Av0D3hpG"
    "5dYDHHkAAAAASUVORK5CYII=")


#----------------------------------------------------------------------
PreFrame = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACQ0lE"
    "QVRIieWVzW4aMRSFbV+DCHTDJlkAYZMKAek2fQHYddElu/SteA+eoNk0iPA7BBBQgtQJI6Gi"
    "ZoQKM2PP2F24P2RmKFGVRauenSX707nH99r4XtfRc4s8O/G/ghKMMcaHoQAAAIdxhFBKGeeM"
    "MR/XDxVCrFYr0zQP4hzH0TStVqvpuk4p3d3waAEA6/W6Wq2m0ul3l5cIISmlDwcAm81mPp83"
    "m827+ZwCFAoFn9NHUEXZWpZt20F3GGPLsiaTidbvz2Yzx3EQQvFkkgSy8kPVeUJ+xQIACjcc"
    "DjudziddZ4ypBIQQyshh6C4OIWSa5sfZrNVqGYZh2zal9GeCUkq19KUUDsUIUUpN0xwOh71e"
    "b2EYnHNCSCQSCe5EgZYKgWKMGefvr64Gt7f3i4XnebvuQhQoP7z5CSG2ZX15ePA8T2W6D+i7"
    "gL1OpZQUoFwuF4vFD9fXo9HI2m6B0lA0CZuU8KJUPelM5u3Jyavz80ajMZ1OXdf9vesDUOXX"
    "c10gpFAsZk5PB4NBvV5fLpdCiF1rT70oH5ozFj86en1x8fLsrN1ud7vdz6sVAKgo1UPxpJby"
    "SQghhEgmk6VSKZ/PN25u+pq2tSzFCsYRMqacc9d1g2jP8xBCqVTqzfFxsVCo1+vj8Zgx9n2u"
    "9kGFEIlEolKpJOJxGeg+Jdd1CSG5XC6bzbbabcMwghOFfR8fxjgajQohOOfhWfyQyvTrZgOE"
    "xGKxXW5I+er5OSghBEboRSIhpfyTi9oniZAMBIr+0o/vH4d+A5itKwKTfnzPAAAAAElFTkSu"
    "QmCC")


#----------------------------------------------------------------------
PreKeyFrame = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACPUlE"
    "QVRIib3Vz0/aYBgH8OfHiyTGgAsciJMl6A5j7OLO4+48eHT/qjflSDPlRweNEqPGrTF2hQwW"
    "gbR9nx2Yi9Ii1bE9x7b55Pu87/Ok+PXqChZdtHDxv6FEsLS0UJQZXVcdHIBSi0OJ0HXV4aHE"
    "QJmZmGOgML99BCAipZTjOPVard/vE00jT2gTEYkIEV3XbZpms9HwfP/T3l4mk9FaPwdVSmmt"
    "v7uuaZqmaV5fX2utX66tEbOIPC0pIjIzANi23basRqPR7XaDIGBmRAQACImPoZNmtda2bbda"
    "raZpOo4zeaiUAgARYebwgUajiJhIJPzR6Jtt12u1k5OTG8f5Y90vVgrjoIQ4Ho/Pz87ax8df"
    "TLM/GABAIpGIbkckVvtE9PP2dn9//8KyiFkp9fvsogoRI99Ohw+CIJ1K7e7ulsvllZWVIAim"
    "xuVBIqVinakAIFE+n8/t7LwtlarVauf0dDgaRUeeMQDRt+95HiG+3tx8lc+32m2jWr24vASA"
    "yfDPCj4HBQAR8X2fmd9vbW0UCs1m8/PRkXNzowH4buUnIxW+qTnDLyKe56VSqQ/l8pti0TCM"
    "Rr3+o9+fuDwjeKw1DYIAAbKZzMft7WKxaBiGZVnD4XDWHcbdfZnQiBuFwvr6eqfTqVQqAhBe"
    "/Blo1Hd3b8T3fSZ6Vyrlcrler7eUTIbzhlBEmLU/92jP816srqbTaREJh8UHv2hEGI9xMJBs"
    "9pG8c+thUhFIJmV5GXz/2SJE/E5E/lKMQhdR/wT9BR51CSZZ1VE7AAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
PrePlay = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACV0lE"
    "QVRIic2Vz1PaUBDHs5tkH9jROv4LDhUHrLXOlOml/p3+H/VYg6YJQz3UWkDDAJlob8J4AUxq"
    "3tse4lBKEIV6cI9vdz7vuz/eW/h1daU9t+GzE18kFBERpxAWhAJAJpO5u7sbDodp7iJQIjIM"
    "o16v7+/vtzsdk2giwJgLZxqGYZqXl5eWZf04O4ui6NPeHqTCngpFRCFEr9f76jjVarXf7wsh"
    "hBBKqXTw41BEFETD21vbtstHR9fX10SUzWaZmZmllHNDiYiZz2q1Q8vyfV9HzGazIy9r2nxQ"
    "0zR1XQ+CwLKsn7WaUkoQAUwWUEqpMT8O1XWdiLrdrm3b305OBoOBEMI0zSk3M7NSWuqmf6BJ"
    "NwaDgeO69vFxt9dLyvdQNolSnqEUAKSU309PLcsKgkDX9dm4ETR9aIyIiFgulz8fHEgpl5eX"
    "ETEtYdyS+iYjBZo2HnoPZWal1IdS6fXqarVaDYKAmWlaZyYsTpQCjLfrb/rM/Gpp6WOp9G57"
    "u1GvO67rBwErNQP9pDlVSt2Goa7r73d3C8Vio9GoVCrtTkc9gAYAfuKLklJKKRFxZ2enUChc"
    "eJ7rOK12W0ppEuEYmpnlXM9UKRWGISK+3drazOc9z6tUKl6z+TuOiWj03ck4ngM6jgaAYrG4"
    "sbHRarVc1/WazTAMiUjTNKnUrDmdYcycqN7M59/kcu1Ox3Hd8/PzMAzjOF7867tXHUUAkMvl"
    "1tfXfd//cniIACqlFBZb0QBARHEc39zcrKysTHgX3FHMHEWRUmptbS3t/a9tyszxtO6/vL3/"
    "kP0BhtFQnDqk9wIAAAAASUVORK5CYII=")

#----------------------------------------------------------------------
Play = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACTUlE"
    "QVRIidXWzU7bQBAH8JmxLW8SbglXLCchTsKFElEUu4aaHnhSnoInoBeEihoSwodEz5wKya67"
    "Oz24UAiOgQhV6p5sy/r5v7OztvHH9TW896B3F/8hats2Ir4zenNzQ0Su6y5M249PENG27YOD"
    "g5+3t192d33fJyKlFDMvjgIAIRrmo6Oj8/PzTqcThWGjXrcsS6WpMWZBlAFs2xZCAMDx8fFg"
    "MAharTAMm82mEEJK+ZrUsygiWkQAQERCCGPMt5OTwelp0Gr1+/0gCBzHUUoVp55FAcCy/178"
    "QzN/HwyGo1Gz0QijqB0EQogCOg8lmll3QnRdl5mHo9HZeNyo1/v9frfbFUKkaaq1fhlFotzC"
    "4T19Nh6PLy58z4vCsLu2VhJCPk2dl9SyCjr0gb64vLy8uvI8b2trKwiCcqn0EOVp8zMDgG1Z"
    "88THtBDCcZzhcLi/v//18JAeFe1J0uxBRAQAzFy8o5RSWuvV1dUkSYJWyxjzkDR/+sUxtdZK"
    "qVq1Gm9vf9zcrFQqUsqimiJiAWqMkVJWKpUoDOM4rtVqSqnJZDJz27OkzEgEzybOzEopIvqw"
    "vp4kied5Wuvn3Bw0L2maptoY3/d3k6Tb6SDidDqdN5s8NGup++OsfMvLy593dnq9XrlUki/t"
    "0fkoIjNPp9OlpaU4jj9FUbValVJOCgMWoUQkpXRdt7exkSTJysrKrzSdV75XoQyAiHXf39vb"
    "a7fbiPgmLhs484kmoru7O8dxsu576zs/P6kxplwuA0Dx+r4NzdyFuWz8Pz8TvwFpng4ClVur"
    "2QAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
NextKeyFrame = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACNUlE"
    "QVRIic2Vy27aQBSGj+3xBQNRlSJlQWhwSKPUSMkQmcu+Sl8zW56AVfMCTQgLVEQSbxCELhFi"
    "wcUzc7qwihQ8NiTNImdny/P5/89tlNFwCO8d6rsTPyAUEXQdCAHE7VBFUXaCmqZ2d6c9PIBh"
    "bIEi4nK5VFVVVbeYQMMg9/fq4yNGoGRDI2Os2+0yziuVim3bnDGMuFv/P87+JhQBRs/PnU7H"
    "9/1Go+E4jqHrnPNYtCzIxrMCYOg6IPZ6vcFgUHbdWr1+mM8DAOf8jdC1ZELIYrH4dXv75PtV"
    "z6OUfs7lBOdCiFdDETFgLLQalms6nf68ufnd63med3F+nk6nhRDJaEn18eUBTdNUVR2Px61W"
    "q9ls9vt9xhghJKHzJDmNRpgNRHzy/eFo5LpurVr9Ui4rMW0XsQ8QZy1Er1ardrvt+75bq32f"
    "TD4dHES/lNgPGJNC12wAmM/nf8bj2WwmFSuvvjTC+pim+bVUopSeXl5mr6+5TMFO0DXupFSi"
    "lJaOj9OZDLMsBUA6ErKWWq3g31oRQnDObdt2HKdRrxcKBdu2OedBEERHPklpOJGhumwmUzo5"
    "oZQWj44syxJCBEGw1ZkEyjhnjNm2/e3s7ILSYrFo6LoQgiUXMBmaSqUqlHrV6mE+b1kWY2x3"
    "nASKiLqu/7i6yu7tWaYZ5i7pNGJ070mgmqblcrldzRICmhZ9rbz9ilZVZTIBQjCbhZdD+Irm"
    "3wwhcH8fECEy1v8BBYCYtf3R7v34+AuNHRpDa7trrwAAAABJRU5ErkJggg==")

#----------------------------------------------------------------------
NextFrame = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACOUlE"
    "QVRIidWVS2/aQBSFZ8ZjYxkTFmSBRFAgpk1CJTIgE/orq/wMJFbNAjbNLqsgxAKVh+INgrhL"
    "HkK8PI8urCKCx01bsWju+vrTuef6noHj0Qgcu9DRie8dCiE8MlQIsdlsEEIIvT0EQkhV1WAn"
    "PtBIKe10OpSxYrFoGAajVAghJUIIF4vFdDqNx+OxWIxzLlcKIRQAjF9eGo1GrVYbDAaMc4yx"
    "1BBN03q93pe7u3a7HdG0UKUAAAiApqpAiG63OxwOP+Xzt5XKWSoFAGCMHTRzzj3P29coh+4k"
    "Y4zX6/VTs/nsOGXbJoQkTk85Y/sICCFCKDjHIVQI4VHqu+ivazabfXt4+N7t2rZ9UyhEo1HO"
    "eVDdfkm2L15/oCgKQsh13Xq9Xq1W+/0+pTTMaLlSaaPvhhDi2XFG43E+n78tlz9eXob9doHx"
    "AQgbzUdvt9tWq+U4TqlUAkJIuXJPpdAdGwCwWq1+uK5pmkhmgnz70vL3E4lEPlgWIaRQKDw1"
    "m/Tx8R+hO1zOsggh1sVF1DQxxmGbko2/3YJfscI5Z4wZhpHNZj9XKul02jAMxpjneb8JB4lS"
    "/9h9dTHTtHI5Qkjm/FzXdf+Egs1vQyljlFLDMK6vrm4IyWQymqpyzmlggUIIznmQC/ffKITQ"
    "crn8en+PFcUul89SKV3XaUhQIYQmk4nruslkMpFIvDrffagfffP5PHZyokcijLGw3NtxMcaM"
    "sYOsgcHXVFEU6VB/XhJPgxH3t/WuX9P/FPoTFHYunoRw1IsAAAAASUVORK5CYII=")

#----------------------------------------------------------------------
LastFrame = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAAClElE"
    "QVRIie2VT0sbQRjG950/Gw8mW4QgCWxaMSdjwkq60YDmYD+A/QBiP5keq2IuHuKxxUNFqoI2"
    "UuOlInahHpYkHhLdmXmnh5VNiLFS8CL4nhZm3t/zzvMsM/D76sp47iLPTnyFvkQoABDyqAYh"
    "BAD+DwoA3W632WwyxiilA5u01r7vCyE454+B+iXvoZTSVru9Va1++fq10+2aphltAgAA+H5w"
    "sL6xcXl5yRhjjD2Eaq0HoWHzzc1NrVZbXV09OjoytOacR2iNeHp6ura2tr297ft+LBaLvOKc"
    "Hx4efl5fD+UNw+hpIiIhhHPued5WtXpycjI/P5+dnARChBBaa9M0AyG+7e01zs/nZmeLxWIi"
    "kZBSEkJarZbneRGqB9WIiAgAjDGt9c+zs18XF47jVBYWkskkIhqGQQgxTbPZatV2dn7U65VK"
    "JTc1xTkf8GSIO6EVpmlKKff39xuNhuu6t7e3kRWMUoNSz/M2Nzez2eyHxUWlVH9QLKIopcJx"
    "en4TwhjzfX93d/eNZVFK+9NgjAkh6vX63d2dlUjQvt+x96W17u9BxCAIGGMzMzPLy8uZTEZK"
    "Ga1KKYUQ4+PjH5eWPq2sJJNJqdSQ4yOi1toAQEQp5cjISCGfL5VK7yYmCMDx8XEoHKLTqZTr"
    "uvl83rIsxphSKgiCYVCtpZQiCOLxuOM4ruu+zWQopUqpQIhwNM65bdvvi8VcLmdZllQqCAJE"
    "TKVS07ncEKhSKmaaJdedK5cztg0AIQgAtNaolG3b5XI5Pz09OjoqhIhGk1IWCgXHcYQQ9wmF"
    "zwkhpN1udzqddDodhjbg75/r67GxsUQ8LqUcyPNhQfRGEUIIIQO43okYQ8QncQ88/WdPf/RP"
    "1ku8pF+hz1V/AVdHWFTfbzsRAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
Key = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAACl0lE"
    "QVRIie2Uv0sbYRjH3x+X9/JGk9wlYkg6icbJMf0LHCxo/QuKUCfR0TFqB/dKHSztYKHBYHCp"
    "kIIFqZM6ODho3Y2tQ44EMRffu3tz79vhlcNGC5I4+h2/L3yen+8Df19cgKcWenLiM7RbKITw"
    "vql1g6OUAikhQowxIUTw1GGmEEKdkEKh8GZq6v3KCgAAY9wttKen58Pqan5hwTCMcrm8sLhI"
    "COkKSgg5Ozv79PlzcWPjy/r6j50du9HY3d2llHYO1XV9s1R6NTY2OjpqWRbGeG5u7uDwMGjr"
    "LRRCiB4tz/N+nZ6OT0y4rgsAcBwnm82GNK1WqyGEQDB9IYTv+wAAhBDGmHOuaZqUUpl3hRBq"
    "NBp2szk0OOh5HgBASkkpTSSTVcsyTVMIoakeHR8fb5ZKGsYN2242m5l0ulqtYozNROLurqia"
    "GGPX19eGYUgplSmljEQiruOotdUAAJzz4eHh+fl5Sun3cvnn3t7y8nI+n0/29c3MzDRtuz1T"
    "256dnXUcJxKJBJG45yGEVBhNxSGE6LpOKTUMIxqNJpPJWCwWi8USpklCoTZoPB4nhPy5vOzv"
    "71f98X2/Vq8bhqHKQkH+QgghBOfccRwhhOu6nucFfiDf9ymlqVTqYH8/HA4DADDGlmVdXV2l"
    "Uql/oEq+72cymZe5HGNsZGQkOzTEOW8blCpwYnz8a6FQr9ej0ahpmltbWy8ymd7eXvUK2y4/"
    "xhhj7LouIURKyTm/fzIQQhDC15OTmqa9W1o6Ojr6tr1dLBaNeLzVaj0AfaR0Xa9UKm+np09O"
    "TgYGBj6ureVyOcbY7dw6g0opw+Fw8+amcn6eTqdN02SMBTV1CFVCCIVCoVar1fZHOr+nAAC1"
    "JA8E6wb6Pz1Dn15/AQsQSZkYzgBNAAAAAElFTkSuQmCC")

#----------------------------------------------------------------------
Stop = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAAAvklE"
    "QVRIie3VMQ6DMAwF0G9CEUxMzAwch4NwCEbuxyXoygiBkMQd2KoUM7Cg4tFSnixZ/qH3MODq"
    "ii4XHzQO9IiQpvJTIlgLY06gRFgW3/dwDkRHqHMoiqiq4L2ExjGPo+k6TBOUOkK1VnUdtS2M"
    "AfMhug+bZfBeQAEkSbB9n+0/6IPeAQ2dKTO0htbi7QcjKoQ6hzx/NQ22TUgpa6ksYe1XmgCg"
    "wMe3B4pYe56u64lJATBjnmX0d91n+3+OfgDGM0GplgNFhQAAAABJRU5ErkJggg==")
#----------------------------------------------------------------------
DeleteKey = PyEmbeddedImage(
    "iVBORw0KGgoAAAANSUhEUgAAABwAAAAcCAIAAAD9b0jDAAAAA3NCSVQICAjb4U/gAAAEXUlE"
    "QVRIibWWX0hUWRzHf+ece4/3TjPjvTMOrqM+lH9KmQhEszCC2OifLRRF+LKwb8X66Eug0UOP"
    "Cz22+7JIGwxmEGmTWUahkhapLQSbwQZFm6iDOubM3DPn3nvOPozT6jhKQft7u/f+zuf3O7/v"
    "+f3ORf98+ADf2vA3JxaC4q8Mg9DGJeufpUSWBYryxSlhcBy0spLHXQ9VFOXxY/z2LWjal+Qo"
    "CaGxGI7HgZDNoQDOwYPqyAianQVV3ZopNY0+eCBDIXfXLrDtzaGuK02THztGYzFIp/PiryPq"
    "Oh0f34ax0tq6jRC81fYBgHNRWens30/v3AEorJvUNPXNG/39+98TiR/b2q5evQoAZE0GBdYg"
    "xpxIRFRV0f5+qaqA0LrPqorn5oqnpn55966zq8vw+WL37nVdukQp3QoKAMiy7JYW6fHQoSG5"
    "VjRCgDHfw4d/VlT81tMTvX69u7v7weBgcmXl0aNHuq5vBc3my48eRcvLyvPnMuuNEGBM+/pw"
    "S0t0bOz4oUPfHz4cj8cJIe3t7WPj40KIdVCEEM4zhLAQzqlT6vS0Oj0Nug6aVjQ4iCoqUvX1"
    "f01Otp48mclkAIAxVlNToyrKwsJCVrFVqBCCc845dxxHSsk5F0II2+aOk2xtxcPD7uwsfvbM"
    "tizrwIHEx4/JVKq6qopzDgBSSk3TAsHgfDyelUsBAErpy5cve27eVAhZSSZTqVS4rGx+fp4Q"
    "YgYCLsZFyeQPg4NzlD7dvh2mpqx0+tOnT4ZhSClXz4OUHo8nwxhCaBVq23ZtbW1HR4eu6/di"
    "scdPnly5cqWzszNYUnLh/Pkk57C0FIhGeSjUdOYMwXhlefnn9nbGmMfj+Vw9m3OMcTYMzsah"
    "lAZMM2CahmH4fL5gMOj3+/1+fyAUCup6eHQULlzQmpu/GxkxQqGKykpK6ceZGSU3JVzXXVhc"
    "NAwjqxX+nL8Q2TLajDEhRCaT4bYtAZTbtzMNDSIUshsaXK8XDwxohlFaWjr29KmmaQBACInH"
    "44lEorS0dB30c8BwONzU2GhZViQSqY5E0N27orzc2bMHGEOM8SNHUCqljI+3nj37x40bi4uL"
    "Pp/PNM1bt26Vh8Nerze7fZQ3+QkhhJBMJkOLi8noqDszw0+fRozlegUDgNbbK5qaTly8qNj2"
    "5cuXX7x4caevLxqNGsXFjuMUgK5Wo6hIef1amZxkbW1ICMipvNpUluXv6/u7vv6nrq5XExPb"
    "d+z49dq1xsZGy7JWdSsAVVU0N0cHBjLnzoGug+sWcJif9w4NJU6ceL+0VFZSYpqmZVkoNyU2"
    "tCkhkE7T+/f58ePS6y1ABADbFmVlqX37vAMDddXVXr+f5U5oIShCAED7+53mZlFejjgvQMw6"
    "Mubu3GnX1cneXjfX8ptAVVUdHhaVlc7u3f+JsxnXsuy9e2VJiTIxAWvmHuTXFCG0vCwNAzYE"
    "39QwRum01PW1Yubfpl9HBAAh8ogboABfR8ylkvfif/lD+Rcv7QbV/D7nwgAAAABJRU5ErkJg"
    "gg==")

class TimeSlider(wx.Window):
    """
    This is the Time Slider Panel.
    """
    def __init__(self, parent, slidersize, sliderStartFrame, sliderEndFrame, curFrame):
        wx.Window.__init__(self, parent, size = slidersize, style = wx.SUNKEN_BORDER)

        self._mainDialog = wx.GetTopLevelParent(self)
        self._mouseIn = False
        self.points = []
        self.numbers = []

        self.curFrame = curFrame
        self.sliderStartFrame = sliderStartFrame
        self.sliderEndFrame = sliderEndFrame
        self.frameNum = self.sliderEndFrame-self.sliderStartFrame+1

        self.InitBuffer()

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)

    def InitBuffer(self):
        self.w,self.h = self.GetClientSize()
        self.buffer = wx.EmptyBitmap(self.w, self.h)
        dc = wx.BufferedDC(wx.ClientDC(self), self.buffer)
        self.DrawTimeSlider(dc)
        self.DrawNumber(dc)
        self.DrawFrame(dc)
        self.DrawKeys(dc)

    def SetTimeSliderData(self, sliderStartFrame = 1, sliderEndFrame = 24, curFrame = 0):
        self.curFrame = curFrame
        self.sliderStartFrame = sliderStartFrame
        self.sliderEndFrame = sliderEndFrame
        self.frameNum = self.sliderEndFrame-self.sliderStartFrame+1

        self.points = []
        self.numbers = []

        self.InitBuffer()
        self.Refresh()

    def OnPaint(self, evt):
        dc = wx.BufferedPaintDC(self, self.buffer)

    def DrawTimeSlider(self,dc):

        dc.SetBackground(wx.Brush(self.GetBackgroundColour()))
        dc.Clear()

        dc.SetPen(wx.BLACK_PEN)
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        self.unitWidth = self.w/float(self.frameNum)

        if self.frameNum <= 20:

            self.points.append(((float(0),self.h),(float(0),self.h-15)))

            for i in range(1,self.frameNum):
                temp = self.points[i-1][0][0]+self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(self.frameNum):
                self.numbers.append(self.sliderStartFrame+i)

            for i in range(self.frameNum):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        elif self.frameNum <= 70:

            self.points.append(((self.unitWidth,self.h),(self.unitWidth,self.h-15)))

            for i in range(1,int((self.frameNum+1)/2)):
                temp = self.points[i-1][0][0]+2*self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(1,self.frameNum/2+1):
                self.numbers.append(self.sliderStartFrame-1+i*2)

            for i in range(int((self.frameNum+1)/2)):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])

            for i in range(self.frameNum/2):
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        elif self.frameNum <= 150:

            self.points.append(((self.unitWidth*4.0,self.h),(self.unitWidth*4.0,self.h-15)))

            for i in range(1,int(self.frameNum/5)):
                temp = self.points[i-1][0][0]+5*self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(1,self.frameNum/5+1):
                self.numbers.append(self.sliderStartFrame-1+i*5)

            for i in range(int(self.frameNum/5)):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])

            for i in range(self.frameNum/5):
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        elif self.frameNum <= 250:

            self.points.append(((self.unitWidth*9.0,self.h),(self.unitWidth*9.0,self.h-15)))

            for i in range(1,int(self.frameNum/10)):
                temp = self.points[i-1][0][0]+10*self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(1,self.frameNum/10+1):
                self.numbers.append(self.sliderStartFrame+i*10)

            for i in range(int(self.frameNum/10)):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])

            for i in range(self.frameNum/10):
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        elif self.frameNum <= 1000:

            self.points.append(((self.unitWidth*49.0,self.h),(self.unitWidth*49.0,self.h-15)))

            for i in range(1,int(self.frameNum/50)):
                temp = self.points[i-1][0][0]+50*self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(1,self.frameNum/50+1):
                self.numbers.append(self.sliderStartFrame-1+i*50)

            for i in range(int(self.frameNum/50)):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])

            for i in range(self.frameNum/50):
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        elif self.frameNum <= 2000:

            self.points.append(((self.unitWidth*99.0,self.h),(self.unitWidth*99.0,self.h-15)))

            for i in range(1,int(self.frameNum/100)):
                temp = self.points[i-1][0][0]+100*self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(1,self.frameNum/100+1):
                self.numbers.append(self.sliderStartFrame-1+i*100)

            for i in range(int(self.frameNum/100)):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])

            for i in range(self.frameNum/100):
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        elif self.frameNum <= 10000:

            self.points.append(((self.unitWidth*999.0,self.h),(self.unitWidth*999.0,self.h-15)))

            for i in range(1,int(self.frameNum/1000)):
                temp = self.points[i-1][0][0]+1000*self.unitWidth
                self.points.append(((temp,self.h),(temp,self.h-15)))

            for i in range(1,self.frameNum/1000+1):
                self.numbers.append(self.sliderStartFrame-1+i*1000)

            for i in range(int(self.frameNum/1000)):
                dc.DrawLine(self.points[i][0][0], self.points[i][0][1], self.points[i][1][0], self.points[i][1][1])

            for i in range(self.frameNum/1000):
                st = str(self.numbers[i])
                tw,th = dc.GetTextExtent(st)
                dc.DrawText(st, self.points[i][0][0]+2, 0.5)

        else:
            pass


    def DrawNumber(self, dc):
        dc.SetPen(wx.BLACK_PEN)
        dc.SetBrush(wx.BLACK_BRUSH)
        dc.SetFont(wx.Font(10, wx.SWISS, wx.NORMAL, wx.NORMAL))

        i = self.curFrame-self.sliderStartFrame
        st = str(self.curFrame)
        tw,th = dc.GetTextExtent(st)
        dc.DrawText(st, float(self.unitWidth)*float(i)+2, self.h-th-0.5)

    def DrawFrame(self, dc):
        i = self.curFrame-self.sliderStartFrame
        pos = float(self.unitWidth)*float(i)
        self.curRect = wx.Rect(pos, float(0), self.unitWidth, self.h)

        oldPen, oldBrush, oldMode = dc.GetPen(), dc.GetBrush(), dc.GetLogicalFunction()

        gray = wx.Colour(200, 200, 200)
        grayPen = wx.Pen(gray)
        grayBrush = wx.Brush(gray)
        dc.SetPen(grayPen)
        dc.SetBrush(grayBrush)
        dc.SetLogicalFunction(wx.XOR)
        dc.DrawRectangleRect(self.curRect)

        dc.SetPen(oldPen)
        dc.SetBrush(oldBrush)
        dc.SetLogicalFunction(oldMode)

    def DrawKeys(self, dc):
        if len(self._mainDialog.keys) != 0:
            for key in self._mainDialog.keys:
                keyFrame = key
                i = keyFrame-self.sliderStartFrame
                pos = float(self.unitWidth)*float(i)

                oldPen, oldBrush, oldMode = dc.GetPen(), dc.GetBrush(), dc.GetLogicalFunction()

                dc.SetPen(wx.Pen("red"))
                dc.SetBrush(wx.Brush("red"))
                dc.SetLogicalFunction(wx.AND)
                dc.DrawLine(pos, float(0), pos, self.h)

                dc.SetPen(oldPen)
                dc.SetBrush(oldBrush)
                dc.SetLogicalFunction(oldMode)
        else:
            pass

    def OnSize(self,evt):
        self.InitBuffer()

    def OnLeftDown(self,evt):
        point = (evt.GetX(), evt.GetY())

        if point[1]>= float(0) and point[1]<= (float(self.h)-2.0):
            if point[0]>= float(0) and point[0]<= float(self.w):
                self._mouseIn = True

        if self._mouseIn:
            self.CaptureMouse()
            self.curFrame = int(float(point[0])/self.unitWidth)+self.sliderStartFrame
            self._mainDialog.curFrame = self.curFrame
            self._mainDialog.curFrameSpin.SetValue(self.curFrame)
            self._mainDialog.OnAnimation(self.curFrame)
            self.SetTimeSliderData(self.sliderStartFrame, self.sliderEndFrame, self.curFrame)

    def OnLeftUp(self,evt):
        if self.GetCapture():
            self.ReleaseMouse()
            self._mouseIn = False

    def OnMotion(self,evt):
        self._mouseIn = False
        if evt.Dragging() and evt.LeftIsDown():
            point = (evt.GetX(), evt.GetY())
            if point[1]>= float(0) and point[1]<= (float(self.h)-2.0):
                if point[0]>= float(0) and point[0]<= float(self.w):
                    self._mouseIn = True

            if self._mouseIn:
                self.curFrame = int(float(point[0])/self.unitWidth)+self.sliderStartFrame
                self._mainDialog.curFrame = self.curFrame
                self._mainDialog.curFrameSpin.SetValue(self.curFrame)
                self._mainDialog.OnAnimation(self.curFrame)
                self.SetTimeSliderData(self.sliderStartFrame, self.sliderEndFrame, self.curFrame)

        evt.Skip()
        self._mouseIn = False

class TimeRange(wx.Window):
    """
    This is the Time Range Panel.
    """
    def __init__(self, parent, rangesize, startFrame, endFrame, sliderStartFrame, sliderEndFrame):
        wx.Window.__init__(self, parent, size = rangesize, style = wx.SUNKEN_BORDER)

        self._mainDialog = wx.GetTopLevelParent(self)

        self._mouseIn = False

        self.startFrame = startFrame
        self.endFrame = endFrame
        self.sliderStartFrame = sliderStartFrame
        self.sliderEndFrame = sliderEndFrame
        self.frameNum = self.endFrame-self.startFrame+1

        self.InitBuffer()

        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_LEFT_DOWN, self.OnLeftDown)
        self.Bind(wx.EVT_LEFT_UP, self.OnLeftUp)
        self.Bind(wx.EVT_MOTION, self.OnMotion)

    def InitBuffer(self):
        self.w,self.h = self.GetClientSize()
        self.buffer = wx.EmptyBitmap(self.w, self.h)
        dc = wx.BufferedDC(wx.ClientDC(self), self.buffer)
        self.DrawTimeRange(dc)

    def SetTimeRangeData(self, startFrame = 1, endFrame = 24, sliderStartFrame = 1, sliderEndFrame = 24):
        self.startFrame = startFrame
        self.endFrame = endFrame
        self.sliderStartFrame = sliderStartFrame
        self.sliderEndFrame = sliderEndFrame
        self.frameNum = self.endFrame-self.startFrame+1

        self.InitBuffer()
        self.Refresh()

    def OnPaint(self, evt):
        dc = wx.BufferedPaintDC(self, self.buffer)

    def DrawTimeRange(self,dc):
        dc.SetBackground(wx.Brush(wx.Colour(150, 150, 150)))
        dc.Clear()

        dc.SetPen(wx.Pen(self.GetBackgroundColour()))
        dc.SetBrush(wx.Brush(self.GetBackgroundColour()))

        self.unitWidth = (self.w-6.0)/float(self.frameNum)
        self.rangePosX = 3.0+(float(self.sliderStartFrame-self.startFrame))*self.unitWidth
        self.rangePosY = 2.0
        self.rangeWidth = float(self.sliderEndFrame-self.sliderStartFrame+1)*self.unitWidth
        self.rangeHeight = self.h-4.0
        self.curRect = wx.Rect(self.rangePosX, self.rangePosY, self.rangeWidth, self.rangeHeight)

        dc.DrawRoundedRectangleRect(self.curRect, radius = 2)

    def OnSize(self,evt):
        self.InitBuffer()

    def OnLeftDown(self,evt):
        point = (evt.GetX(), evt.GetY())
        self.pos = 0

        if point[1]>= self.rangePosY and point[1]<= self.rangePosY+self.rangeHeight:
            if point[0]>= self.rangePosX and point[0]<= self.rangePosX+self.rangeWidth:
                self._mouseIn = True

        if self._mouseIn:
            self.CaptureMouse()
            self.pos = point

    def OnLeftUp(self,evt):
        if self.GetCapture():
            self.ReleaseMouse()
            self._mouseIn = False

    def OnMotion(self,evt):
        self._mouseIn = False
        if evt.Dragging() and evt.LeftIsDown():
            newPos = (evt.GetX(), evt.GetY())
            if newPos[1]>= self.rangePosY and newPos[1]<= self.rangePosY+self.rangeHeight:
                if newPos[0]>= self.rangePosX and newPos[0]<= self.rangePosX+self.rangeWidth:
                    self._mouseIn = True

            if self._mouseIn:
                if newPos[0] == self.pos[0]:
                    evt.Skip()
                    self._mouseIn = False
                if newPos[0] > self.pos[0]:
                    if float(newPos[0]-self.pos[0]) >= self.unitWidth:
                        if self.sliderEndFrame < self.endFrame:
                            self.sliderStartFrame += 1
                            self.sliderEndFrame += 1
                            self.SetTimeRangeData(self.startFrame, self.endFrame, self.sliderStartFrame, self.sliderEndFrame)
                            self.MainPanelUpdate()
                            self.pos = newPos
                            evt.Skip()
                            self._mouseIn = False
                if newPos[0] < self.pos[0]:
                    if float(self.pos[0]-newPos[0]) >= self.unitWidth:
                        if self.sliderStartFrame > self.startFrame:
                            self.sliderStartFrame -=1
                            self.sliderEndFrame -=1
                            self.SetTimeRangeData(self.startFrame, self.endFrame, self.sliderStartFrame, self.sliderEndFrame)
                            self.MainPanelUpdate()
                            self.pos = newPos
                            evt.Skip()
                            self._mouseIn = False
        evt.Skip()
        self._mouseIn = False

    def MainPanelUpdate(self):
        self._mainDialog.sliderStartFrame = self.sliderStartFrame
        self._mainDialog.sliderEndFrame = self.sliderEndFrame
        self._mainDialog.timeSliderStartSpin.SetValue(self.sliderStartFrame)
        self._mainDialog.timeSliderEndSpin.SetValue(self.sliderEndFrame)
        self._mainDialog.timeSlider.SetTimeSliderData(self._mainDialog.sliderStartFrame, self._mainDialog.sliderEndFrame, self._mainDialog.curFrame)


class AnimControlUI(wx.Dialog):
    """
    This is the Animation Control main class implementation.
    """
    def __init__(self, parent, editor):
        wx.Dialog.__init__(self, parent, id=wx.ID_ANY, title="Animation Controller",
                           pos=wx.DefaultPosition, size=(920, 110))

        self.editor = editor
        self._initOver = False

        self.parallel = []
        if self.editor.animMgr.keyFramesInfo != []:
            self.editor.animMgr.generateKeyFrames()
        self.keys = self.editor.animMgr.keyFrames
        self.editor.objectMgr.findActors(render)
        self.editor.objectMgr.findNodes(render)

        self.prePlay = False
        self.play = False
        self.stop = True

        self.curFrame = 1
        self.startFrame = 1
        self.sliderStartFrame = 1
        self.endFrame = 24
        self.sliderEndFrame = 24

        self.mainPanel1 = wx.Panel(self, -1)

        self.timeSlider = TimeSlider(self.mainPanel1, wx.Size(560, 35), self.sliderStartFrame, self.sliderEndFrame, self.curFrame)
        self.curFrameSpin = wx.SpinCtrl(self.mainPanel1, -1, "",size = (70,25),min=self.startFrame, max=self.endFrame)

        bmpFirstFrame = FirstFrame.GetBitmap()
        bmpPreFrame = PreFrame.GetBitmap()
        bmpPreKeyFrame = PreKeyFrame.GetBitmap()
        self.bmpPrePlay = PrePlay.GetBitmap()
        self.bmpPlay = Play.GetBitmap()
        bmpNextKeyFrame = NextKeyFrame.GetBitmap()
        bmpNextFrame = NextFrame.GetBitmap()
        bmpLastFrame = LastFrame.GetBitmap()
        bmpKey = Key.GetBitmap()
        self.bmpStop = Stop.GetBitmap()
        bmpDeleteKey = DeleteKey.GetBitmap()

        self.buttonFirstFrame = wx.BitmapButton(self.mainPanel1, -1, bmpFirstFrame, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonPreFrame = wx.BitmapButton(self.mainPanel1, -1, bmpPreFrame, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonPreKeyFrame = wx.BitmapButton(self.mainPanel1, -1, bmpPreKeyFrame, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonPrePlay = wx.BitmapButton(self.mainPanel1, -1, self.bmpPrePlay, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonPlay = wx.BitmapButton(self.mainPanel1, -1, self.bmpPlay, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonNextKeyFrame = wx.BitmapButton(self.mainPanel1, -1, bmpNextKeyFrame, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonNextFrame = wx.BitmapButton(self.mainPanel1, -1, bmpNextFrame, size = (30,30),style = wx.BU_AUTODRAW)
        self.buttonLastFrame = wx.BitmapButton(self.mainPanel1, -1, bmpLastFrame, size = (30,30),style = wx.BU_AUTODRAW)

        self.mainPanel2 = wx.Panel(self, -1)

        self.timeStartSpin = wx.SpinCtrl(self.mainPanel2, -1, "",size = (70,25),min=0, max=self.sliderEndFrame)
        self.timeSliderStartSpin = wx.SpinCtrl(self.mainPanel2, -1, "",size = (70,25),min=self.startFrame, max=self.sliderEndFrame)
        self.timeRange = TimeRange(self.mainPanel2, wx.Size(450, 25), self.startFrame, self.endFrame, self.sliderStartFrame, self.sliderEndFrame)
        self.timeSliderEndSpin = wx.SpinCtrl(self.mainPanel2, -1, "",size = (70,25),min=self.sliderStartFrame, max=self.endFrame)
        self.timeEndSpin = wx.SpinCtrl(self.mainPanel2, -1, "",size = (70,25),min=self.sliderStartFrame, max=10000)
        self.buttonDeleteKey = wx.BitmapButton(self.mainPanel2, -1, bmpDeleteKey, size = (30,30),style = wx.BU_AUTODRAW)

        self.SetProperties()
        self.DoLayout()

        self.Bind(wx.EVT_SPINCTRL, self.OnCurrentTime, self.curFrameSpin)

        self.Bind(wx.EVT_BUTTON, self.OnFirstFrame, self.buttonFirstFrame)
        self.Bind(wx.EVT_BUTTON, self.OnPreFrame, self.buttonPreFrame)
        self.Bind(wx.EVT_BUTTON, self.OnPreKeyFrame, self.buttonPreKeyFrame)
        self.Bind(wx.EVT_BUTTON, self.OnPrePlay, self.buttonPrePlay)
        self.Bind(wx.EVT_BUTTON, self.OnPlay, self.buttonPlay)
        self.Bind(wx.EVT_BUTTON, self.OnNextKeyFrame, self.buttonNextKeyFrame)
        self.Bind(wx.EVT_BUTTON, self.OnNextFrame, self.buttonNextFrame)
        self.Bind(wx.EVT_BUTTON, self.OnLastFrame, self.buttonLastFrame)

        self.Bind(wx.EVT_SPINCTRL, self.OnTimeStartSpin, self.timeStartSpin)
        self.Bind(wx.EVT_SPINCTRL, self.OnTimeSliderStartSpin, self.timeSliderStartSpin)
        self.Bind(wx.EVT_SPINCTRL, self.OnTimeSliderEndSpin, self.timeSliderEndSpin)
        self.Bind(wx.EVT_SPINCTRL, self.OnTimeEndSpin, self.timeEndSpin)

        self.Bind(wx.EVT_BUTTON, self.OnDeleteKey, self.buttonDeleteKey)

        self.Bind(wx.EVT_CLOSE, self.OnExit)

        self.OnPropKey()
        self.OnAnimation(self.curFrame)

        self.timeUnit = float(1)/float(24)*float(1000)
        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.OnTimer, self.timer)

        self._initOver = True

    def SetProperties(self):
        self.curFrameSpin.SetValue(self.curFrame)
        self.timeStartSpin.SetValue(self.startFrame)
        self.timeSliderStartSpin.SetValue(self.sliderStartFrame)
        self.timeSliderEndSpin.SetValue(self.sliderEndFrame)
        self.timeEndSpin.SetValue(self.endFrame)

    def DoLayout(self):
        dialogSizer = wx.BoxSizer(wx.VERTICAL)
        mainSizer1 = wx.FlexGridSizer(1, 10, 0, 0)
        mainSizer2 = wx.FlexGridSizer(1, 6, 0, 0)

        mainSizer1.Add(self.timeSlider, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
        mainSizer1.Add(self.curFrameSpin, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 3)

        mainSizer1.Add(self.buttonFirstFrame, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonPreFrame, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonPreKeyFrame, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonPrePlay, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonPlay, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonNextKeyFrame, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonNextFrame, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)
        mainSizer1.Add(self.buttonLastFrame, 0, wx.ALIGN_CENTER_VERTICAL)

        mainSizer2.Add(self.timeStartSpin, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer2.Add(self.timeSliderStartSpin, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer2.Add(self.timeRange, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer2.Add(self.timeSliderEndSpin, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer2.Add(self.timeEndSpin, 0, wx.ALIGN_CENTER_VERTICAL|wx.LEFT|wx.RIGHT, 5)
        mainSizer2.Add(self.buttonDeleteKey, 0, wx.ALIGN_CENTER_VERTICAL|wx.RIGHT, 3)

        self.mainPanel1.SetSizerAndFit(mainSizer1)
        self.mainPanel2.SetSizerAndFit(mainSizer2)

        dialogSizer.Add(self.mainPanel1, 1, wx.ALIGN_CENTER_VERTICAL|wx.TOP, 5)
        dialogSizer.Add(self.mainPanel2, 1, wx.ALIGN_CENTER_VERTICAL|wx.TOP, 7)

        self.SetSizer(dialogSizer)
        self.Layout()

        self.dialogSizer = dialogSizer

    def OnCurrentTime(self, evt):
        self.curFrame = evt.GetInt()
        self.timeSlider.SetTimeSliderData(self.sliderStartFrame, self.sliderEndFrame, self.curFrame)
        self.OnAnimation(self.curFrame)

    def OnControl(self):
        self.curFrameSpin.SetValue(self.curFrame)
        self.timeSlider.SetTimeSliderData(self.sliderStartFrame, self.sliderEndFrame, self.curFrame)
        self.OnAnimation(self.curFrame)

    def OnFirstFrame(self,evt):
        self.curFrame = self.sliderStartFrame
        self.OnControl()

    def OnPreFrame(self,evt):
        if self.curFrame-1 >= self.startFrame:
            self.curFrame -= 1
            self.OnControl()
        else:
            evt.Skip()

    def sortKey(self):
        for i in range(0, len(self.keys)-1):
            for j in range(i+1, len(self.keys)):
                if self.keys[i]>self.keys[j]:
                    temp = self.keys[i]
                    self.keys[i] = self.keys[j]
                    self.keys[j] = temp

    def OnPreKeyFrame(self,evt):
        self.sortKey()
        if self.curFrame <= self.keys[0] or self.curFrame > self.keys[len(self.keys)-1]:
            self.curFrame = self.keys[len(self.keys)-1]
        else:
            for i in range(1, len(self.keys)):
                if self.curFrame <= self.keys[i] and self.curFrame > self.keys[i-1]:
                    self.curFrame = self.keys[i-1]
                    break
        self.OnControl()

    def OnTimer(self,evt):
        if self.prePlay == True and self.stop == False and self.play == False:
            if self.curFrame-1>=self.sliderStartFrame:
                self.curFrame -= 1
                self.OnControl()
            elif self.curFrame == self.sliderStartFrame:
                self.curFrame = self.sliderEndFrame
                self.OnControl()

        if self.play == True and self.stop == False and self.prePlay == False:
            if self.curFrame+1<=self.sliderEndFrame:
                self.curFrame += 1
                self.OnControl()
            elif self.curFrame == self.sliderEndFrame:
                self.curFrame = self.sliderStartFrame
                self.OnControl()

    def OnPrePlay(self,evt):
        if self.prePlay == False and self.stop == True and self.play == False:
            self.buttonPrePlay = wx.BitmapButton(self.mainPanel1, -1, self.bmpStop, size = (30,30),style = wx.BU_AUTODRAW)
            self.DoLayout()
            self.prePlay = True
            self.stop = False
            self.timer.Start(self.timeUnit)
            evt.Skip()

        elif self.prePlay == True and self.stop == False and self.play == False:
                self.buttonPrePlay = wx.BitmapButton(self.mainPanel1, -1, self.bmpPrePlay, size = (30,30),style = wx.BU_AUTODRAW)
                self.DoLayout()
                self.prePlay = False
                self.stop = True
                self.timer.Stop()
                evt.Skip()
        else:
            evt.Skip()

    def OnPlay(self,evt):
        if self.play == False and self.stop == True and self.prePlay == False:
            self.buttonPlay = wx.BitmapButton(self.mainPanel1, -1, self.bmpStop, size = (30,30),style = wx.BU_AUTODRAW)
            self.DoLayout()
            self.play = True
            self.stop = False
            self.timer.Start(self.timeUnit)
            evt.Skip()

        elif self.play == True and self.stop == False and self.prePlay == False:
                self.buttonPlay = wx.BitmapButton(self.mainPanel1, -1, self.bmpPlay, size = (30,30),style = wx.BU_AUTODRAW)
                self.DoLayout()
                self.play = False
                self.stop = True
                self.timer.Stop()
                evt.Skip()
        else:
            evt.Skip()

    def OnNextKeyFrame(self,evt):
        self.sortKey()
        if self.curFrame < self.keys[0] or self.curFrame >= self.keys[len(self.keys)-1]:
            self.curFrame = self.keys[0]
        else:
            for i in range(0, len(self.keys)-1):
                if self.curFrame >= self.keys[i] and self.curFrame < self.keys[i+1]:
                    self.curFrame = self.keys[i+1]
                    break
        self.OnControl()

    def OnNextFrame(self,evt):
        if self.curFrame+1 <= self.endFrame:
            self.curFrame += 1
            self.OnControl()
        else:
            evt.Skip()

    def OnLastFrame(self,evt):
        self.curFrame = self.sliderEndFrame
        self.OnControl()

    def OnTime(self):
        preFrame = self.curFrame
        self.curFrameSpin.SetRange(self.startFrame,self.endFrame)
        self.curFrame = preFrame
        self.timeSlider.SetTimeSliderData(self.sliderStartFrame, self.sliderEndFrame, self.curFrame)
        self.timeRange.SetTimeRangeData(self.startFrame, self.endFrame, self.sliderStartFrame, self.sliderEndFrame)
        self.parallel = self.editor.animMgr.createParallel(self.startFrame,self.endFrame)

    def OnTimeStartSpin(self,evt):
        self.startFrame = evt.GetInt()
        self.timeSliderStartSpin.SetRange(self.startFrame, self.sliderEndFrame)
        if self.startFrame >= self.sliderStartFrame:
            self.sliderStartFrame = self.startFrame
            self.timeSliderStartSpin.SetValue(self.sliderStartFrame)
            self.OnTime()
        else:
            self.OnTime()

    def OnTimeSliderStartSpin(self,evt):
        self.sliderStartFrame = evt.GetInt()
        self.timeEndSpin.SetRange(self.sliderStartFrame, 10000)
        self.OnTime()

    def OnTimeSliderEndSpin(self,evt):
        self.sliderEndFrame = evt.GetInt()
        self.timeStartSpin.SetRange(0, self.sliderEndFrame)
        self.OnTime()

    def OnTimeEndSpin(self,evt):
        self.endFrame = evt.GetInt()
        self.timeSliderEndSpin.SetRange(self.sliderStartFrame, self.endFrame)
        if self.endFrame <= self.sliderEndFrame:
            self.sliderEndFrame = self.endFrame
            self.timeSliderEndSpin.SetValue(self.sliderEndFrame)
            self.OnTime()
        else:
            self.OnTime()

    def OnDeleteKey(self,evt):
        for i in range(0,len(self.keys)):
            if self.curFrame == self.keys[i]:
                del self.keys[i]
                break

        for j in list(self.editor.animMgr.keyFramesInfo.keys()):
            for k in range(0,len(self.editor.animMgr.keyFramesInfo[j])):
                if self.curFrame == self.editor.animMgr.keyFramesInfo[j][k][0]:
                    del self.editor.animMgr.keyFramesInfo[j][k]
                    break

        for l in list(self.editor.animMgr.keyFramesInfo.keys()):
            if len(self.editor.animMgr.keyFramesInfo[l]) == 0:
                del self.editor.animMgr.keyFramesInfo[l]

        self.OnPropKey()
        self.OnAnimation(self.curFrame)

    def OnPropKey(self):
        self.parallel = self.editor.animMgr.createParallel(self.startFrame, self.endFrame)
        self.timeSlider.SetTimeSliderData(self.sliderStartFrame, self.sliderEndFrame, self.curFrame)


    def OnAnimation(self, curFrame):
        time = float(curFrame-1)/float(24)
        self.parallel.setT(time)
        if self.editor.GRAPH_EDITOR == True:
            self.editor.ui.graphEditorUI.curFrameChange()

    def OnExit(self,evt):
        for actor in self.editor.objectMgr.Actor:
            actorAnim = os.path.basename(actor[OG.OBJ_ANIM])
            actor[OG.OBJ_NP].loop(actorAnim)
        self.parallel = None
        self.Destroy()
        self.editor.ui.editAnimMenuItem.Check(False)
        self.editor.mode = self.editor.BASE_MODE







