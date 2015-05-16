import subprocess
from urllib import urlopen
import os
import time

NO_ROUND = "none"
ONGOING = "ONGOING_ROUND_CODE"
SAVE_FILE = "eda-bot-save.txt"

def getRound(i): 
    ret =  NO_ROUND
    page = urlopen("https://old.jutge.org/competitions/AlbertAtserias:BolaDeDrac2015/round/" + str(i)).read()
    found = page.find("Player out:")

    if found != -1:
        inici = page.find("<p class='indent'>", found+1)
        fi =  page.find("(",inici+1)

        if (inici != -1 and fi != -1) :
            ret =  page[inici+18:fi]
     
    elif page.find("Turn 1") != -1 :
        ret = ONGOING

    return ret

def readLastRound() :
    n = 1
    print "READING LAST ROUNDS"
    while (getRound(n) !=  NO_ROUND) :
        print getRound(n)
        n = n + 1

    return n-1



def sendTwit(dead, n) :
    print dead + " is out (" + str(n) + ")"  
    twit = "Round " + str(n) + ": " + dead + " is out."
    command = "twitter set " + twit
    subprocess.call(command.split(), shell=False)
    print "SENDING TWIT: " + twit

def sendOngoingTwit(n) :
    #print dead + " is out (" + str(n) + ")"  
    twit = "Round " + str(n) + " ongoing..."
    command = "twitter set " + twit
    subprocess.call(command.split(), shell=False)
    print "SENDING TWIT: " + twit


lastRound = 18  #readLastRound()
ongoingRound = -1

print "Last round: " + str(lastRound)

while True :
    res = getRound(lastRound + 1)
    print "Round " + str(lastRound + 1) + " returned: ";
    if res == NO_ROUND:
        print  "     NO_ROUND"
    elif res == ONGOING:
        print  "     ONGOING"
    else:
        print res

    if res !=  NO_ROUND :
        if res == ONGOING and lastRound + 1 != ongoingRound :
            ongoingRound = lastRound+1
            sendOngoingTwit(ongoingRound)
      
        if res != ONGOING :
            ongoingRound = -1
            lastRound = lastRound+1
            sendTwit(res, lastRound)
           
      
    else :
        if ongoingRound == -1:
            time.sleep(10);
        else:
            time.sleep(3);
