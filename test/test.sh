curl "http://localhost:8085/?method=createUser&usrId=t1"
curl "http://localhost:8085/?method=createUser&usrId=t2"
curl "http://localhost:8085/?method=createUser&usrId=t3"
curl "http://localhost:8085/?method=addSubscription&usrId=t1&subscribeTo=t2"
curl "http://localhost:8085/?method=getSubscription&usrId=t1"
curl "http://localhost:8085/?method=getSubscription&usrId=t1"
curl "http://localhost:8085/?method=getSubscription&usrId=t1"
curl "http://localhost:8085/?method=postTwit&usrId=t2&content=aa"
curl "http://localhost:8085/?method=postTwit&usrId=t2&content=bb"
curl "http://localhost:8085/?method=postTwit&usrId=t2&content=gg"
curl "http://localhost:8085/?method=postTwit&usrId=t3&content=cc"
curl "http://localhost:8085/?method=getTwitsBySubscription&usrId=t1"


