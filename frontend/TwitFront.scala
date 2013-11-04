package com.example

import scala.util.{Success, Failure}
import akka.actor.Actor
import spray.routing._
import spray.http._
import MediaTypes._
import HttpMethods._
import spray.can.Http
import com.redis._
import spray.client.pipelining._
import scala.concurrent._
import akka.actor.ActorSystem
import scala.util.control.Breaks._


class TwitContent(usrId : String , postTime : Long , content : String) {
  def this(srzString : String) = {
    this(srzString.split(";")(0),(srzString.split(";")(1)).toLong,srzString.split(";")(2))
  }

  def mkSrz() : String = usrId + ";" + postTime.toString() + ";" + content
}


class TwitFront extends Actor {

  def actorRefFactory = context

  def print_client_response(res : String) = {
    println(res)
  }
  def construct_id(rawId : String , t : String) : String = {
    val prefix = 
      if(t=="user") "userId-"
      else if(t=="subTo") "subTo-"
      else if(t=="subToCount") "subToCount-"
      else if(t=="twitCount") "twitCount-"
      else if(t=="twitId") "twitId-"

    prefix+rawId
    
  }
 
  def create_user(usrId : String , rc : RedisClient): Unit = {
    val rUsrId = construct_id(usrId , "user")
    println("real user id: " + rUsrId)

    val v = TwitStore.get(rUsrId)
    
    if(v=="") {
      println("User Not found")
      TwitStore.set(rUsrId , "welcome")
    }
    else{
      println("User exists: " + usrId + v)
    } 
  }

  def addSub(usrId : String, subToId : String) : Unit = {
    if(subToId==""){
      println("add subscription: subscribeToId cannot be empty")
    }
    else{
      val rUsrId = construct_id(usrId , "user")
      val subToCountId = construct_id(usrId , "subToCount")

      val subToCount = TwitStore.get(subToCountId)
      val newSubToCount = 
        if(!subToCount.forall(_.isDigit) 
          || subToCount=="") 1 else subToCount.toInt+1
      val rawSubToId = usrId + "--" + newSubToCount
      val rSubToId = construct_id(rawSubToId , "subTo")

      TwitStore.incr(subToCountId)
      TwitStore.set(rSubToId , subToId)
      
    }
  }

/*
  def removeSubscription(usrId : String, subscribeToId : String , rc : RedisClient) : Unit = {
    val rUsrId = construct_id(usrId , "subscribeTo")
    rc.srem(rUsrId , subscribeToId)
  }
*/
  def getSubscription(usrId : String) : List[String] = {
    val rUsrId = construct_id(usrId , "subscribeTo")
   
    val subToCountId = construct_id(usrId , "subToCount")
    val subToCount = TwitStore.get(subToCountId)
    if(!subToCount.forall(_.isDigit)) Nil
    else{
      for{
        i<-(1 to subToCount.toInt).toList
        preSubToCountId = usrId + "--" + i
        rSubToId = construct_id(preSubToCountId , "subTo")
        subTo = TwitStore.get(rSubToId)
        if(subTo != "")
      }
      yield subTo
    }

  }

  def postTwit(usrId : String , contents : String) : Unit = {
    val rUsrId = construct_id(usrId , "user")
    if(rUsrId.isEmpty){
      println("user not exist, twit not posted")
    }
    else {
      val twitCountId = construct_id(usrId , "twitCount")
      val twitCount = TwitStore.get(twitCountId)
      val newTwitCount = if(!twitCount.forall(_.isDigit) || twitCount=="") 1 else twitCount.toInt+1
      val rawTwitId = usrId + "--" + newTwitCount
      val rTwitId = construct_id(rawTwitId , "twitId")
      val newTwit = new TwitContent(usrId , System.currentTimeMillis/1000 , contents)
      TwitStore.incr(twitCountId)
      TwitStore.set(rTwitId , newTwit.mkSrz)
      println("twit posted: "+ rTwitId + " : " + newTwit.mkSrz())
    }
  }

  def getTwits(usrId : String) : List[TwitContent] = {
    val twitCountId = construct_id(usrId , "twitCount")
    val twitCount = TwitStore.get(twitCountId)
    if (!twitCount.forall(_.isDigit)) Nil
    else{
      println("twitCount: " + twitCount)
      for {
        i <- (1 to twitCount.toInt).toList
        preTwitId = usrId + "--" + i
        rTwitId = construct_id(preTwitId , "twitId")
        void = println("get twit : " + rTwitId)
        twit = TwitStore.get(rTwitId)
        void1 = println(twit)
        if(twit!="")
      }
    
      yield new TwitContent(twit)
    }
   
  }

  def getTwitsBySubscription(usrId : String) : List[TwitContent] = {
    val rUsrId = construct_id(usrId , "user")
    if(TwitStore.get(rUsrId)==""){
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

    if("createUser"==method.get) create_user(usrId.get , rc)
    else if("postTwit"==method.get) postTwit(usrId.get , content.get)
    else if("getTwits"==method.get) println(getTwits(usrId.get))
    else if("addSubscription"==method.get) addSub(usrId.get , subscribeTo.get)
    else if("getSubscription"==method.get) println(getSubscription(usrId.get))
    else if("getTwitsBySubscription"==method.get) println(getTwitsBySubscription(usrId.get))
    else if("clearDB"==method.get) rc.flushdb
  }

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
