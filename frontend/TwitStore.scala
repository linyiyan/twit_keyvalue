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
