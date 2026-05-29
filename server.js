const express = require("express");

const app = express();

app.use(express.json());

app.get("/", (req, res) => {
    console.log('get');
});

app.post("/rfid", (req, res) => {

  console.log("UID:", req.body.uid);

    res.json({
        success: true,
        message: "Selamat datang"
    });
});

app.listen(3000, () => {
  console.log("Server running");
});