(defun map (l f) (if (= l nil) nil (cons (f (car l)) (map (cdr l) f))))
(defun twice (n) (* n 2))
(defun id (n) (n))
(print (map (quote 1 2 3) twice))
