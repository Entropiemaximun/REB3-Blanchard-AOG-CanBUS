# REB3-Blanchard-AOG-CanBUS
 catch  information from REB3  sprayer controler to paint the AOG map
 
  very bad  code  ( sorry in advance )
  modification  of  code  https://github.com/mtz8302/AOG_SectionControl_ESP32
 adding CAN  function  ( request  librairie : https://github.com/autowp/arduino-mcp2515 ) 
  
  
  
  video  available  here :
  https://youtu.be/h_2Xe81FQ_0
  
  0:05 Hardware part
inspiration https://github.com/Xilofeon/Control_Amatron
0:50 Software part
0:55 Canhacker Installation
link: http://mictronics.de/posts/USB-CAN-Bus/
1:35 Initial Arduino program and library installation
link https://github.com/autowp
link https://github.com/coryjfowler
manual library link https://github.com/autowp/arduino-canhacker/tree/master/docs/en
4:40 Can-hacker: sniffing to decode useful frames
9:54 Modifying an arduino program to read and send info to AOG
inspiration https://github.com/mtz8302/AOG_SectionControl_ESP32
14:40 Uploading Arduino code
15:25 Closing Arduino IDE and launching AOG
16:24 Coloring in AOG

first attempt to read the CAN bus
  
  ------------------------------------------
  
  
  # REB3-Blanchard-AOG-CanBUS
Recuperation  des information du REB3  controleur du pulverisateur pour peindre la carte dans AgopenGPS
 
 programme assez pourri (desole  d'avance )  
  inspiration du  code  https://github.com/mtz8302/AOG_SectionControl_ESP32
 Ajout  d'une fonction CAN  bus ( besoin de la librairie : https://github.com/autowp/arduino-mcp2515 ) 
    
  
  video  available  here :
  https://youtu.be/h_2Xe81FQ_0
  
  0:05 Partie matériel
inspiration  https://github.com/Xilofeon/Control_Amatron
0:50 Partie logiciel
0:55 Can-hacker Installation
lien: http://mictronics.de/posts/USB-CAN-Bus/
1:35 Programme Arduino initial et installation de librairie
lien https://github.com/autowp
lien https://github.com/coryjfowler
lien librairie manuel https://github.com/autowp/arduino-canhacker/tree/master/docs/en
4:40  Can-hacker: reniflage pour décoder les trames utile
9:54  Modification d'un programme arduino  pour lire et envoyer l’info à AOG
inspiration https://github.com/mtz8302/AOG_SectionControl_ESP32
14:40 Téléversement du code Arduino 
15:25  Fermeture Arduino IDE  et  lancement AOG
16:24   Coloriage dans AOG

premier essai de lire le bus CAN
