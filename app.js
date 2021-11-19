require('dotenv').config();

//Basic server setup requirements
var bodyParser = require("body-parser");
const express = require('express');
const nodemailer = require('nodemailer')

//Basic MongoDB setup and connections
const MongoClient = require('mongodb').MongoClient;
const assert = require('assert');

//getting the Database URL from the environment file
const uri = process.env.DATABASE_URL;

const app = express();

var http = require('http');
var path = require("path");

app.use(express.urlencoded({
    extended: false
}));

//Define that we are using json objects
app.use(express.json());

//create a http server that supports websocket
const server = http.createServer(app); 
//==========================================================

require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    console.log('addr: ' + add);
})

//websocket creation and setup.
const WebSocket = require('ws');
const s = new WebSocket.Server({
    server
});

//Email configurations 
const transporter = nodemailer.createTransport({
    //defining the email service
    service: "gmail",
    auth: {
        user:   'greensgomail@gmail.com',
        //Sensitive data is retrieved from the .env file
        pass:   process.env.PASSWORD
    }
})


//===============================================================================================

//Event that only triggers when a Node device connects to the server the entire server waits until a connection is made.
s.on('connection', function (ws, req) {

    //when server receives message from client this function triggers. "message" refers to the data from the Node
    //with a single string format, values are separated by commas. 
    ws.on('message', function (message) {
            console.log("new client connected");
            
            let rawData = message.split(',');
            let temp = rawData[0];
            let humi = rawData[1];
            let mois = rawData[2];
            let light = rawData[3];

            //=================================
            //Math to convert any data exceeding 100 back to base 100
            let moisPer = Math.round(((mois*100)/780))
            //Math to convert any data exceeding 100 back to base 100
            let lightPer = Math.round(((light*100)/960))

            if (lightPer > 100) {
                lightPer = 100
            }

            if (moisPer > 100) {
                moisPer = 100
            }
            //==================================
            
            //Time instance that created according to the device's location "South Africa" in this instance to ensure that
            //server location does not influence the time gotten.
            let time = new Date(Date.now()).toLocaleString('en-ZA', { timeZone: 'Africa/Harare' });

            var ticker = new Date(Date.now())
            let _time = ticker.getHours() + ":" + ticker.getMinutes() + ":" + ticker.getSeconds();

            //This function checks whether the time is 6pm so that a mail message can be generated to send a daily summary
            if (_time == '18:00:00') {

                const mailOptions = {
                    from: 'greensgomail@gmail.com',
                    to: 'gustafdelport@gmail.com',
                    subject: 'Sensor Report',
                    text: `Time: ${time}\n\nTemperature: ${temp} Celsuis\nHumidity: ${temp}%\nMoisture: ${moisPer}%\nLight: ${lightPer}%`
                }
                
                //The mail configurations is used to send messages to all the subscribers.
                transporter.sendMail(mailOptions,(err,info) => {
                    if (err) {
                        console.log(err);
                    } 
                    else {
                        console.log('Email sent: ' + info.response);
                    } 
                })
            }
            
            //This is responsible for connecting and sending the data that was gotten form the Node to mongoDB
            MongoClient.connect(uri, function(err,client) {
                assert.equal(null,err);
                const db = client.db("myFirstDatabase");

                db.collection('datas').insertOne({
                    //JSON obj
                    Time: time,
                    Temperature: temp,
                    HumidityPercentage: humi,
                    MoisturePercentage: moisPer,
                    LightIndex: lightPer
                })
                client.close();
            })
    });

    //This event will listen whether the client that is currently connected disconnects and the server will remain idle 
    //until a new Node connects
    ws.on('close', function () {
        console.log("lost one client");
    });
});

server.listen(3000);