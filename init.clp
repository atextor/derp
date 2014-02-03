(deftemplate example
	(multislot v)
	(slot w (default 9))
	(slot x)
	(slot y)
	(multislot z))

(defrule duck "the duck rule"
	(animal-is duck)
	=>
	(assert (sound-is quack)))


