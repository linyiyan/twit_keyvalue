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
import scala.concurrent.duration._
import akka.actor.ActorSystem
import scala.util.control.Breaks._



object TwitStore{
  implicit val system = ActorSystem()
  import system.dispatcher // execution context for futures

  val pipeline: HttpRequest => Future[HttpResponse] = sendReceive
  val base_uri = "http://localhost:8080/"

  def get(key : String) : String = {
    val uri = base_uri + "?method=get"+"&key="+key
    val response: HttpResponse = Await.result(pipeline(Get(uri)) , 1000 millis)
    response.entity.asString.trim()
  }

  def set(key : String , value : String) : Int = {
    val uri = base_uri + "?method=set"+"&key="+key+"&value="+value
    // Await.ready(pipeline(Get(uri)) , 1000 millis)
    val response: HttpResponse = Await.result(pipeline(Get(uri)) , 1000 millis)
    response.status.intValue
  }


  def incr(key : String) : Int = {
    val uri = base_uri + "?method=incr"+"&key="+key
    val response: HttpResponse = Await.result(pipeline(Get(uri)) , 1000 millis)
    response.status.intValue
  }
}
