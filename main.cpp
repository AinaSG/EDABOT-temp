#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <unistd.h>

#include "twitcurl.h"
#include "oauthlib.h"


#define HEADER_ACCEPT "Accept:text/html,application/xhtml+xml,application/xml"
#define HEADER_USER_AGENT "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.70 Safari/537.17"
#define SAVE_FILE "eda-bot-save.txt"
#define NO_ROUND "AJddAHasfBJbgajFbIfFrDdHkBbcSdRWDghOpJb"
#define ONGOING "AJddAHsdgnwahDsdgFSAsdgDbgajFbIfFrDdHkBbcSdRWDghOpJb"
using namespace std;

curlpp::Easy request;
ostringstream responseStream;
twitCurl twitterObj;
std::string tmpStr, tmpStr2;
std::string replyMsg;
char tmpBuf[1024];


void curlpp_init() {
  list<string> headers;
  headers.push_back(HEADER_ACCEPT);
  headers.push_back(HEADER_USER_AGENT);
  request.setOpt(new curlpp::options::HttpHeader(headers));
  request.setOpt(new curlpp::options::FollowLocation(true));
  request.setOpt(curlpp::options::SslVerifyPeer(false));
  request.setOpt(curlpp::options::SslVerifyHost(false));

  curlpp::options::WriteStream streamWriter(&responseStream);
  request.setOpt(streamWriter);
}

void save_last_round(int last) {
  ofstream osav;
  osav.open(SAVE_FILE);
  osav <<  last;
  osav.close();
}

string get_round(int i) {
  string ret =  NO_ROUND;
  char buff[100];
  sprintf(buff, "https://old.jutge.org/competitions/AlbertAtserias:BolaDeDrac2015/round/%d", i);
  string url = buff;
  request.setOpt(curlpp::options::Url(url));

  request.perform();
  string re = responseStream.str();
  size_t found = re.find("Player out:");

  if (found != string::npos) {
    size_t inici = re.find("<p class='indent'>",found+1);
    size_t fi =  re.find("(",found+1);
    cout << " --> ";
    if (inici != string::npos and fi != string::npos) {
      ret =  re.substr(inici+18, fi-(inici+18));
      cout << ret << endl;
    }
  } else if (re.find("Turn 1") != string::npos) {
    ret = ONGOING;
  }

  responseStream.str(string());
  return ret;
}

int read_last_round() {
  ifstream isav(SAVE_FILE);
  int n;
  if (isav.good()) {
    isav >>  n;
  }  else {
    n = 1;
    while (get_round(n) !=  NO_ROUND) ++n;
    save_last_round(n);
  }
  isav.close();
  return n;
}

void printUsage()
{
  printf( "\nUsage:\neda-bot -u username -p password\n" );
}

void send_twit(string dead, int n) {
  cout << dead << " is out (" << n << ")" << endl;  
  sprintf(tmpBuf, "Round %d: %s is out.", n, dead.c_str());
  tmpStr = tmpBuf;
  replyMsg = "";
  if( twitterObj.statusUpdate( tmpStr ) )
  {
    cout << "twit correcte" << endl;
  }
  else
  {
    cout << "el twit ha fallat" << endl;
  }
}


void send_ongoing_twit(int n) { 
  cout << "ongoing (" << n << ")" << endl;
  sprintf(tmpBuf, "Round %d ongoing...", n);
  tmpStr = tmpBuf;
  replyMsg = "";
  if( twitterObj.statusUpdate( tmpStr ) )
  {
    cout << "twit correcte" << endl;
  }
  else
  {
    cout << "el twit ha fallat" << endl;
  }
}


int main( int argc, char* argv[] )
{
    /* Get username and password from command line args */
  std::string userName( "" );
  std::string passWord( "" );
  if( argc > 4 )
  {
    for( int i = 1; i < argc; i += 2 )
    {
      if( 0 == strncmp( argv[i], "-u", strlen("-u") ) )
      {
        userName = argv[i+1];
      }
      else if( 0 == strncmp( argv[i], "-p", strlen("-p") ) )
      {
        passWord = argv[i+1];
      }
    }
    if( ( 0 == userName.length() ) || ( 0 == passWord.length() ) )
    {
      printUsage();
      return 0;
    }
  }
  else
  {
    printUsage();
    return 0;
  }

    /* Set twitter username and password */
  twitterObj.setTwitterUsername( userName );
  twitterObj.setTwitterPassword( passWord );

    /* OAuth flow begins */
    /* Step 0: Set OAuth related params. These are got by registering your app at twitter.com */
  twitterObj.getOAuth().setConsumerKey( std::string( "vlC5S1NCMHHg8mD1ghPRkA" ) );
  twitterObj.getOAuth().setConsumerSecret( std::string( "3w4cIrHyI3IYUZW5O2ppcFXmsACDaENzFdLIKmEU84" ) );

    /* Step 1: Check if we alredy have OAuth access token from a previous run */
  std::string myOAuthAccessTokenKey("");
  std::string myOAuthAccessTokenSecret("");
  std::ifstream oAuthTokenKeyIn;
  std::ifstream oAuthTokenSecretIn;

  oAuthTokenKeyIn.open( "twitterClient_token_key.txt" );
  oAuthTokenSecretIn.open( "twitterClient_token_secret.txt" );

  memset( tmpBuf, 0, 1024 );
  oAuthTokenKeyIn >> tmpBuf;
  myOAuthAccessTokenKey = tmpBuf;

  memset( tmpBuf, 0, 1024 );
  oAuthTokenSecretIn >> tmpBuf;
  myOAuthAccessTokenSecret = tmpBuf;

  oAuthTokenKeyIn.close();
  oAuthTokenSecretIn.close();

  if( myOAuthAccessTokenKey.size() && myOAuthAccessTokenSecret.size() )
  {
        /* If we already have these keys, then no need to go through auth again */
    printf( "\nUsing:\nKey: %s\nSecret: %s\n\n", myOAuthAccessTokenKey.c_str(), myOAuthAccessTokenSecret.c_str() );

    twitterObj.getOAuth().setOAuthTokenKey( myOAuthAccessTokenKey );
    twitterObj.getOAuth().setOAuthTokenSecret( myOAuthAccessTokenSecret );
  }
  else
  {
        /* Step 2: Get request token key and secret */
    std::string authUrl;
    twitterObj.oAuthRequestToken( authUrl );

        /* Step 3: Get PIN  */
    memset( tmpBuf, 0, 1024 );
    printf( "\nDo you want to visit twitter.com for PIN (0 for no; 1 for yes): " );
    gets( tmpBuf );
    tmpStr = tmpBuf;
    if( std::string::npos != tmpStr.find( "1" ) )
    {
            /* Ask user to visit twitter.com auth page and get PIN */
      memset( tmpBuf, 0, 1024 );
      printf( "\nPlease visit this link in web browser and authorize this application:\n%s", authUrl.c_str() );
      printf( "\nEnter the PIN provided by twitter: " );
      gets( tmpBuf );
      tmpStr = tmpBuf;
      twitterObj.getOAuth().setOAuthPin( tmpStr );
    }
    else
    {
            /* Else, pass auth url to twitCurl and get it via twitCurl PIN handling */
      twitterObj.oAuthHandlePIN( authUrl );
    }

        /* Step 4: Exchange request token with access token */
    twitterObj.oAuthAccessToken();

        /* Step 5: Now, save this access token key and secret for future use without PIN */
    twitterObj.getOAuth().getOAuthTokenKey( myOAuthAccessTokenKey );
    twitterObj.getOAuth().getOAuthTokenSecret( myOAuthAccessTokenSecret );

        /* Step 6: Save these keys in a file or wherever */
    std::ofstream oAuthTokenKeyOut;
    std::ofstream oAuthTokenSecretOut;

    oAuthTokenKeyOut.open( "twitterClient_token_key.txt" );
    oAuthTokenSecretOut.open( "twitterClient_token_secret.txt" );

    oAuthTokenKeyOut.clear();
    oAuthTokenSecretOut.clear();

    oAuthTokenKeyOut << myOAuthAccessTokenKey.c_str();
    oAuthTokenSecretOut << myOAuthAccessTokenSecret.c_str();

    oAuthTokenKeyOut.close();
    oAuthTokenSecretOut.close();
  }
    /* OAuth flow ends */

    /* Account credentials verification */
  if( twitterObj.accountVerifyCredGet() )
  {
    twitterObj.getLastWebResponse( replyMsg );
    printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet web response:\n%s\n", replyMsg.c_str() );
  }
  else
  {
    twitterObj.getLastCurlError( replyMsg );
    printf( "\ntwitterClient:: twitCurl::accountVerifyCredGet error:\n%s\n", replyMsg.c_str() );
  }

  curlpp_init();

  int last_round =  read_last_round();
  int ongoing_round = -1;

  cout << "Last round: " << last_round << endl;
  while(true) {
    string res = get_round(last_round + 1);
    cout << "Round " << last_round + 1 << " returned ";
    if (res == NO_ROUND) cout <<  "NO_ROUND" << endl;
    else if (res == ONGOING) cout <<  "ONGOING" << endl;
    else cout <<  res << endl;

    if (res !=  NO_ROUND) {
      if (res == ONGOING and last_round+1 != ongoing_round) {
        ongoing_round = last_round+1;
        send_ongoing_twit(ongoing_round);
      }
      if (res != ONGOING) {
        ongoing_round = -1;
        ++last_round;
        send_twit(res, last_round);
        save_last_round(last_round);
      }
    } else {
      if (ongoing_round == -1)
        sleep(10);
      else
        sleep(3);
    }
  }

  return 0;
}
