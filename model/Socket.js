// const mongoose = require('mongoose')

import mongoose from 'mongoose'

const socketSchema = new mongoose.Schema({
   
    name:{
        type: String,
        required: true,
        trim:true
    },
   
    isActive:{
        type: Boolean,
        default: false
    }
});
export default mongoose.model("Socket", socketSchema);