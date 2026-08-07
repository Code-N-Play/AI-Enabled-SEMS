import Socket from "../../model/Socket.js";

class SocketController {
  static async create(req, res) {
    try {
      const socket = await Socket.create({
        name: "socket2",
        isActive: false,
      });

      res.status(201).json(socket);
    } catch (err) {
      res.status(500).json({
        message: err.message,
      });
    }
  }

  static async getAll(req, res) {
    try {
      const sockets = await Socket.find();

      res.status(200).json(sockets);
    } catch (err) {
      res.status(500).json({
        message: err.message,
      });
    }
  }

  static async show(req, res) {
    try {
      const socket = await Socket.findOne({_id:req.params.id});

      res.status(200).json(socket);
    } catch (err) {
      res.status(500).json({
        message: err.message,
      });
    }
  }

  static async updateStatus(req, res) {
    try {
      const { id } = req.params;
      const { isActive } = req.body;

      const socket = await Socket.findByIdAndUpdate(
        id,
        { isActive },
        { new: true }
      );

      if (!socket) {
        return res.status(404).json({
          success: false,
          message: "Socket not found",
        });
      }

      res.status(200).json({
        success: true,
        message: "Socket status updated successfully",
        data: socket,
      });
    } catch (err) {
      res.status(500).json({
        success: false,
        message: err.message,
      });
    }
  }
}

export default SocketController;