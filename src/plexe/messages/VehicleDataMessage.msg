import ManeuverMessage;

packet VehicleDataMessage extends ManeuverMessage {
    int senderPlatoonId;
    int senderX;
    int senderVelocity;
    double platoonSize;
}