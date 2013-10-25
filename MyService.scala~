package com.example

import akka.actor.Actor
import spray.routing._
import spray.http._
import MediaTypes._
import HttpMethods._
import spray.can.Http
import com.redis._


class TwitContent(usrId : String , postTime : Long , content : String) {
  def this(srzString : String) = {
    this(srzString.split(";")(0),(srzString.split(";")(1)).toLong,srzString.split(";")(2))
  }

  def mkSrz() : String = usrId + ";" + postTime.toString() + ";" + content
}

// we don't implement our route structure directly in the service actor because
// we want to be able to test it independently, without having to spin up an actor
class MyServiceActor extends Actor with MyService {

  // the HttpService trait defines only one abstract member, which
  // connects the services environment to the enclosing actor or test
  def actorRefFactory = context

  val rc = new RedisClient("localhost", 6379)


  def construct_id(rawId : String , t : String) : String = {
    val prefix = 
      if(t=="user") "userId#"
      else if(t=="subscribeTo") "subscribeTo#"
      else if(t=="twitCount") "twitCount#"
      else if(t=="twitId") "twitId#"

    prefix+rawId
    
  }
 
  def create_user(usrId : String , rc : RedisClient): Unit = {
    val rUsrId = construct_id(usrId , "user")
    println("real user id: " + rUsrId)
    val v = rc.get(rUsrId)
    if(v==None) {
      println("User Not found")
      rc.set(rUsrId , "welcome")
    }
    else{
      println("User exists: " + usrId + v.get)
    }
  }

  def addSubscription(usrId : String, subscribeToId : String) : Unit = {
    if(subscribeToId==""){
      println("add subscription: subscribeToId cannot be empty")
    }
    else{
      val rUsrId = construct_id(usrId , "subscribeTo")
      rc.sadd(rUsrId , subscribeToId)
    }
  }

  def removeSubscription(usrId : String, subscribeToId : String , rc : RedisClient) : Unit = {
    val rUsrId = construct_id(usrId , "subscribeTo")
    rc.srem(rUsrId , subscribeToId)
  }

  def getSubscription(usrId : String) : List[String] = {
    val rUsrId = construct_id(usrId , "subscribeTo")
    val members = rc.smembers(rUsrId)
    for{mem <- members.get.toList} yield mem.get
  }

  def postTwit(usrId : String , contents : String) : Unit = {
    val rUsrId = construct_id(usrId , "user")
    if(rUsrId.isEmpty){
      println("user not exist, twit not posted")
    }
    else {
      val twitCountId = construct_id(usrId , "twitCount")
      val twitCount = rc.get(twitCountId)
      val newTwitCount = if(twitCount.isEmpty) 1 else twitCount.get.toInt
      val rawTwitId = usrId + "#" + newTwitCount
      val rTwitId = construct_id(rawTwitId , "twitId")
      val newTwit = new TwitContent(usrId , System.currentTimeMillis/1000 , contents)
      rc.incr(twitCountId)
      rc.set(rTwitId , newTwit.mkSrz)

      println("twit posted: "+ rTwitId + " : " + newTwit.mkSrz())
    }
  }

  def getTwits(usrId : String) : List[TwitContent] = {
    val twitCountId = construct_id(usrId , "twitCount")
    val twitCount = rc.get(twitCountId)
    if (twitCount.isEmpty) Nil
    else{
      for {
        i <- (1 to twitCount.get.toInt).toList
        preTwitId = usrId + "#" + i
        rTwitId = construct_id(preTwitId , "twitId")
        void = println("get twit : " + rTwitId)
        twit = rc.get(rTwitId)
        if(!twit.isEmpty)
      }
      yield new TwitContent(twit.get)
    }
   
  }

  def getTwitsBySubscription(usrId : String) : List[TwitContent] = {
    val rUsrId = construct_id(usrId , "user")
    if(rc.get(rUsrId).isEmpty){
      println("get twits by subscription: user not exist")
      Nil
    }
    else{
      val subs = getSubscription(usrId)
      for{sub <- subs
        subTwits = getTwits(sub)
        twit <- subTwits}
      yield twit
    }
  }


  def process_request(r : HttpRequest) : Unit = {
    val method = r.uri.query.get("method")
    val usrId = r.uri.query.get("usrId")
    val subscribeTo = if(method==Some("addSubscription")
							|| method==Some("removeSubscription")) r.uri.query.get("subscribeTo") else Some("")
    val content = if(method==Some("postTwit")) r.uri.query.get("content") else Some("")

    // println("method: " + method.get + " user Id: " + usrId.get)
    if("createUser"==method.get) create_user(usrId.get , rc)
    else if("postTwit"==method.get) postTwit(usrId.get , content.get)
    else if("getTwits"==method.get) println(getTwits(usrId.get))
    else if("addSubscription"==method.get) addSubscription(usrId.get , subscribeTo.get)
    else if("getSubscription"==method.get) println(getSubscription(usrId.get))
    else if("getTwitsBySubscription"==method.get) println(getTwitsBySubscription(usrId.get))
    else if("clearDB"==method.get) rc.flushdb
  }

  // this actor only runs our route, but you could add
  // other things here, like request stream processing
  // or timeout handling
  def receive = {
	case _ : Http.Connected => sender ! Http.Register(self)
	case r@HttpRequest(GET,_,_,_,_) => {
      
      process_request(r)
	      
      sender ! index
	}

  }
  
  lazy val index = HttpResponse(
	entity = HttpEntity(`text/html`,
		<html>
		  <body>
			<h1>Say hello to <i>spray-routing</i> on <i>spray-can</i>!</h1>
		  </body>
		</html>.toString()
	)
  )
}


// this trait defines our service behavior independently from the service actor
trait MyService extends HttpService {

  val myRoute =
    path("") {
      get {
        respondWithMediaType(`text/html`) { // XML is marshalled to `text/xml` by default, so we simply override here
          complete {
            <html>
              <body>
                <h1>Say hello to <i>spray-routing</i> on <i>spray-can</i>!</h1>
              </body>
            </html>
          }
        }
      }
    }
}
