Lutherie augmentée : contrôle actif d’un tom
============================================

COURTE DESCRIPTION
------------------

Le contrôle actif d'un instrument de musique consiste à rajouter une boucle de contrôle à un instrument existant pour en changer en temps réel le comportement.
Dans ce projet, le travail porte sur une percussion à peaux : un tom constitué d’un fût cylindrique en bois sur lequel sont tendues deux peaux, une peau de frappe et une peau de résonance. L’objectif du projet est de concevoir et tester un système de contrôle réalisé en remplaçant la peau de résonance par un haut-parleur. Le haut-parleur est piloté par un calculateur qui transforme le signal de pression capté par un microphone placé dans la cavité de l’instrument. L’algorithme de contrôle implémenté dans le calculateur permet alors de modifier les propriétés sonores de l’instrument.
Plusieurs aspects seront à considérer pour réaliser ce projet, concernant tant la modélisation de la vibration de la peau couplée à la cavité que l’élaboration du contrôleur qui permette de modifier certaines propriétés, comme par exemple la première fréquence de résonance du système. Les aspects liés à la programmation du micro-ordinateur Udoo pour une exécution temps-réel seront aussi à prendre en compte, de même que les caractéristiques des transducteurs électroacoustiques mis en œuvre : haut-parleur et microphone. Préalablement à la mise en œuvre du contrôle sur un instrument réel, la simulation pourra permettre de tester la loi de contrôle élaborée.

OBJECTIFS PEDAGOGIQUES
----------------------

A l’issue de ce projet, vous devrez être capables de :
- présenter un modèle de couplage entre la vibration de la peau et le comportement acoustique dans la cavité, ainsi que les hypothèses sous-jacentes,
- justifier le choix d’une loi de contrôle
- simuler le système, en intégrant les transducteurs électroacoustiques (microphone et haut-parleur) afin de tester la validité du contrôle
- implémenter l’algorithme de contrôle sur un calculateur temps-réel de type Udoo
- mettre en oeuvre le contrôle sur un instrument réel équipé d’un microphone, d’un haut-parleur et d’un calculateur

RESULTATS ATTENDUS
------------------

- Une bibliographie sommaire sur le contrôle actif en général et sur les membranophones
- Une description quasi-analytique des modes de vibration d'une membrane couplée à une cavité d'air
(sans et avec haut-parleur)
- Une simulation (en Python ou Matlab)
- Le développement d'un contrôleur pour le haut-parleur pour obtenir une cible choisie
- une implémentation sur un vrai instrument avec un micro-ordinateur Udoo, des capteur(s), des
actuateur(s)

ENCADRANTS
----------

Brigitte d'Andréa-Novel, Benoît Fabre
Avec l’appui de
Camille Dianoux, Marc Wijnand

BIBLIOGRAPHIE
-------------

[1] Stephen J. Elliott and Philip A. Nelson. Active noise control. IEEE signal processing magazine, 10(4) :12{35, 1993}.

[2] Christopher C Fuller, Sharon Elliott, and Philip A Nelson. Active control of vibration. Academic Press, 1996. Chap 3 (Feedback control)

[3] Laurence Kinsler et al. Fundamentals of acoustics 4th Edition, 2000. Section 4.7 (the kettledrum)

[4] Philip M. Morse. Vibration and sound. Acoustical Society of America, 1995. Sections 19 (circular membrane) and 21 (the kettledrum)