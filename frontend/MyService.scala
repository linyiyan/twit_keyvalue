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

object TwitStore{
  implicit val system = ActorSystem()
  import system.dispatcher // execution context for futures
  val lk = new Lock()

  val pipeline: HttpRequest => Future[HttpResponse] = sendReceive
  val base_uri = "http://localhost:8080/"

  def get(key : String) : String = {
    var status : Int = 0
    var res : String = ""


    def asyncGet(key : String) : Unit = {
      val uri = base_uri + "?method=get"+"&key="+key
      val response: Future[HttpResponse] = pipeline(Get(uri))
      response onComplete {
        case Success(sth) => status=1; res = sth.entity.asString.trim()//.substring(0,sth.entity.asString.indexOf(";"))
        case Failure(_) => status=2
      }
    }
    
   
    asyncGet(key)
    
    breakable{
      while(true){
        if(status !=0 ) break
        Thread.sleep(10)
      }
    }
    
    res
  }

  def set(key : String , value : String) : Unit = {
    var status : Int = 0

    def asyncSet(key : String , value : String) : Unit = {
      val uri = base_uri + "?method=set"+"&key="+key+"&value="+value
      println("send uri " + uri)
      val response: Future[HttpResponse] = pipeline(Get(uri))
        response onComplete {
          case Success(sth) => status=1; println(sth.toString())
          case Failure(_) => status=2; println("client not reponse")
        }
    }

   
    asyncSet(key , value)
   
    breakable{
      while(true){
        if(status!=0) break
        Thread.sleep(10)
      }
    }
   
  }
 
  def incr(key : String) : Unit = {
    var status : Int = 0

    def asyncIncr(key : String) : Unit = {
      val uri = base_uri + "?method=incr"+"&key="+key
      println("send uri " + uri)
      val response: Future[HttpResponse] = pipeline(Get(uri))
      response onComplete {
        case Success(sth) => status=1; println(sth.toString())
        case Failure(_) => status=2; println("client not reponse")
      }
    }


    asyncIncr(key)

    breakable{
      while(true){
        if(status!=0) break
        Thread.sleep(10)
      }

    }
  }
}


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


  def print_client_response(res : String) = {
    println(res)
  }
  def construct_id(rawId : String , t : String) : String = {
    val prefix = 
      if(t=="user") "userId-"
      else if(t=="subscribeTo") "subscribeTo-"
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
      // rc.set(rUsrId , "welcome")
      TwitStore.set(rUsrId , "welcome")
    }
    else{
      println("User exists: " + usrId + v)
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
      // val twitCount = rc.get(twitCountId)
      val twitCount = TwitStore.get(twitCountId)
      val newTwitCount = if(!twitCount.forall(_.isDigit) || twitCount=="") 1 else twitCount.toInt+1
      val rawTwitId = usrId + "--" + newTwitCount
      val rTwitId = construct_id(rawTwitId , "twitId")
      val newTwit = new TwitContent(usrId , System.currentTimeMillis/1000 , contents)
      // rc.incr(twitCountId)
      TwitStore.incr(twitCountId)
      // rc.set(rTwitId , newTwit.mkSrz)
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
        // twit = rc.get(rTwitId)
        twit = TwitStore.get(rTwitId)
        void1 = println(twit)
        if(!twit.isEmpty)
      }
      // yield new TwitContent(twit.get)
      yield new TwitContent(twit)
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

      //implicit val system = ActorSystem()
      //import system.dispatcher // execution context for futures
      //val pipeline: HttpRequest => Future[HttpResponse] = sendReceive
      // val pipeline = sendReceive
      // val responseFuture = Test.getResponse()

      //responseFuture onComplete {
        //case Success(_) => println("client reponse")
      //}
      
      
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
