package com.example

import scala.util.{Success, Failure}
import akka.actor.Actor
import akka.actor.ActorRef
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
 
  def createUser(usrId : String): Boolean = {
    val rUsrId = construct_id(usrId , "user")
    println("real user id: " + rUsrId)

    val v = TwitStore.get(rUsrId)
    
    if(v=="") {
      println("User Not found")
      val status = TwitStore.set(rUsrId , "welcome")
      status == 200 // http response code is 200
    }
    else{
      // println("User exists: " + usrId + v)
      false
    } 
  }

  def addSub(usrId : String, subToId : String) : Boolean = {
    if(subToId==""){
      false
      // println("add subscription: subscribeToId cannot be empty")
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

      val istatus = TwitStore.incr(subToCountId)
      val sstatus = TwitStore.set(rSubToId , subToId)
      
      istatus==200 && sstatus==200
    }
  }

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

  def postTwit(usrId : String , contents : String) : Boolean= {
    val rUsrId = construct_id(usrId , "user")
    if(rUsrId.isEmpty){
      false
    }
    else {
      val twitCountId = construct_id(usrId , "twitCount")
      val twitCount = TwitStore.get(twitCountId)
      val newTwitCount = if(!twitCount.forall(_.isDigit) || twitCount=="") 1 else twitCount.toInt+1
      val rawTwitId = usrId + "--" + newTwitCount
      val rTwitId = construct_id(rawTwitId , "twitId")
      val newTwit = new TwitContent(usrId , System.currentTimeMillis/1000 , contents)
      val istatus = TwitStore.incr(twitCountId)
      val sstatus = TwitStore.set(rTwitId , newTwit.mkSrz)

      istatus==200 && sstatus==200
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


  def process_request(r : HttpRequest , sender : ActorRef) : Unit = {
    val method = r.uri.query.get("method")
    val usrId = r.uri.query.get("usrId")
    val subscribeTo = 
      if(method==Some("addSubscription")|| method==Some("removeSubscription")) 
        r.uri.query.get("subscribeTo") else Some("")
    val content = 
      if(method==Some("postTwit"))
        r.uri.query.get("content") else Some("")

    if("createUser"==method.get) {
      if(createUser(usrId.get)){
        sender ! HttpResponse(entity = HttpEntity(`text/html` , usrId.get + " created\n"))
      }
      else{
        sender ! HttpResponse(entity = HttpEntity(`text/html` , usrId.get + " not created\n"))
      }
    }
    else if("postTwit"==method.get) {
      if(postTwit(usrId.get , content.get)){
        sender ! HttpResponse(entity = HttpEntity(`text/html` , usrId.get + " twit posted: " + content.get + "\n"))
      }
      else
        sender ! HttpResponse(entity = HttpEntity(`text/html` , usrId.get + " twit not posted\n"))
    }
    else if("getTwits"==method.get) {
      val twitLst = getTwits(usrId.get)
      val twitLstStr = twitLst.foldLeft("")((res,twit) => res+"\n"+twit.mkSrz)
      sender ! HttpResponse(entity = HttpEntity(`text/html` , "The twits posted by " + usrId.get + ":\n" + twitLstStr + "\n"))
    }
    else if("addSubscription"==method.get) {
       if(addSub(usrId.get , subscribeTo.get)){
         sender ! HttpResponse(entity = HttpEntity(`text/html` , "subscription from " + usrId.get + "to " + subscribeTo  + " created\n"))
       }
      else{
        sender ! HttpResponse(entity = HttpEntity(`text/html` , "subscription from " + usrId.get + "to " + subscribeTo  + " not created\n"))
      }
    }
    else if("getSubscription"==method.get) {
      val subLst = getSubscription(usrId.get)
      val subLstStr = subLst.foldLeft("")((res,sub) => res + "\n" + sub)

      sender ! HttpResponse(entity = HttpEntity(`text/html` , "Subscription by " + usrId.get + "is :\n"+ subLstStr + "\n"))
    }
    else if("getTwitsBySubscription"==method.get) {
      val twitLst = getTwitsBySubscription(usrId.get)
      val twitLstStr = twitLst.foldLeft("")((res,twit) => res+"\n"+twit.mkSrz)
      
      sender ! HttpResponse(entity = HttpEntity(`text/html` , "The twits of " + usrId.get + " by subscription "  + "are :\n"+ twitLstStr + "\n"))
    }
  }

  def receive = {
	case _ : Http.Connected => sender ! Http.Register(self)
	case r@HttpRequest(GET,_,_,_,_) => {      
      process_request(r , sender)      
      sender ! index
	}
  }
  
  lazy val index = HttpResponse(
	entity = HttpEntity(`text/html`,"Hello")
  )
}
