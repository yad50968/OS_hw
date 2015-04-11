(custom-set-variables
 '(inhibit-startup-screen t))
(custom-set-faces
 )
(setq backup-directory-alist
      `((".*" . ,temporary-file-directory)))
(setq auto-save-file-name-transforms
      `((".*" ,temporary-file-directory t)))

(global-linum-mode t)

(setq sgml-set-face t)
(setq sgml-auto-activate-dtd t)
(setq sgml-indent-data t)
