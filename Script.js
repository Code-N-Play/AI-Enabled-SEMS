import express from 'express'

const app = express()
app.set ("view engine", "ejs");
app.use(express.static('./public'));

app.get('/', (req, res) => {
  res.render("home")
})

app.get('/EnergyMonitoring', (req, res) => {
    res.render("EnergyMonitoring")
})

app.get('/SmartSockets', (req, res) => {
    res.render("SmartSockets")
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

app.listen(3000, () => {
  console.log('Server is running on http://localhost:3000')
})