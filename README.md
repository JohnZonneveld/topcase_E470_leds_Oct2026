# topcase_E470_leds_v2
Made again some changes. Removed the FAT NINJA (compressed 4x3) scrolling animation as startup sequence with an additional double flash at the end. In stead it has been changed to an arrow animation making use of the matrix I created for the animated text.
Removed some obsolete code and added some additional comments to clarify what is happening.
As I am receiving a pulsing input from the turn signal lights and not the state of the turn signal switch I am using a hold routine to keep the turn status active so in between flashes it doesn't return to the running lights (rest condition). As also the turnsignal can be inconsistent I am actually measuring the on/off time to adjust the hold timer when needed so the turn signal is synced to the bike's flash pattern.
State as of now
- Running lights when inputs are no active (at about 80 of 255 brightness).
- Turn Signals synced with bike, turn signal side will show a comet pattern from inside to outside and other side will show running light state ( turn signal at 200 of 255 brightness). 
- Brake Signal, both sides will be fully lit (at about 200 of 255 brightness).
- Brake and Turn active, turning side will show the comet sweep other side will be fully lit (both turn and brake are at 200 of 255 brightness).
- Hazards (both sides will show comet sweep at 200 of 255 brightness).
- Hazards plus brake active, Brake will override hazards, only bike's hazards will show while brake is active. LEDs in topcase will continue with comet sweep at 200 of 255 brightness.
