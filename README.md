# ULCFG Manager Lite

Un programme Homebrew pour PlayStation 2 pour gérer les fichiers UL d'Open PS2 Loader (OPL).

## Fonctionnalités

- **Rebuild ul.cfg** : Scanne mass:/ pour les dossiers ul.*, extrait les GameID depuis gameid.txt, crée ul.cfg propre sans doublons.
- **Merge ul.cfg** : Fusionne tous les fichiers .cfg en un ul.cfg unique, supprime doublons.

## Compilation

Assurez-vous que PS2SDK est installé.

```bash
make
```

Cela produit ulcfg.elf.

## Utilisation

Lancez via wLaunchELF. Utilisez la manette PS2 pour naviguer :
- X : Rebuild
- Carré : Merge
- Cercle : Quitter

## Notes

- Backup automatique de ul.cfg avant modifications.
- Interface texte avec couleurs.
- Taille < 120 KB.