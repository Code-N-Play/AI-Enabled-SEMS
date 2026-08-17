// if (process.env.NODE_ENV !== 'production') {

//   import dotenv from 'dotenv'
//   dotenv.config();
// }
import dotenv from "dotenv";
dotenv.config();

import express from 'express'
import ejsMate from "ejs-mate";
import mongoose from 'mongoose'
// const mongoose = require ("mongoose");
import Socket from "./model/Socket.js";
import SocketController from "./controllers/api/SocketController.js";
 

mongoose.connect(process.env.DATABASE_URL);
const db = mongoose.connection
db.on('error', error => console.error(error));
db.once('open', () => console.log('connected to the user database'));

const app = express()
app.set ("view engine", "ejs");
app.engine("ejs", ejsMate);
app.use(express.static('./public'));
app.use(express.json());
app.use((req, res, next) => { 
  res.locals.currentPath = req.path;  
  next();
});

app.get('/', (req, res) => {
  res.render("home")
});

app.get('/EnergyMonitoring', (req, res) => {
    res.render("EnergyMonitoring")
})

app.get('/SmartSockets', async (req, res) => {
  const sockets = await Socket.find();

    res.render("SmartSockets",{sockets:sockets})
})
  
app.get('/AiInsights', (req, res) => {
      res.render("AiInsights")
})
  
app.get('/Analytics', (req, res) => {
      res.render("Analytics")
})

app.get('/Alerts', (req, res) => {
    res.render("Alerts")
})

app.get('/Settings', (req, res) => {
    res.render("Settings")
})

// create sockets

app.get("/createsocket", async function (req,res) {
  const cretingsocket = await Socket.create({
    name:"ClassRoom 102",
    slug: "socket01"
  });

  res.send(cretingsocket);
});

app.get("/sockets", SocketController.getAll);
app.get("/sockets/create", SocketController.create);
app.get("/sockets/:id", SocketController.show);
app.get("/sockets-update/:slug", SocketController.showViaSlug);
app.patch("/sockets/:slug/status", SocketController.updateStatus);

//https://blynk.cloud/external/api/update?token=Vi0sJxv9D5QOeXdTQY4KraatyMXjbtah&V1=1
//https://blynk.cloud/external/api/get?token=Vi0sJxv9D5QOeXdTQY4KraatyMXjbtah&V1



app.listen(3000,"0.0.0.0", () => {
  console.log('Server is running on http://localhost:3000')
})