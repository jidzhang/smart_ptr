#!/bin/bash

echo "============================================"
echo "Clean Build Artifacts"
echo "============================================"
echo ""

FILES_DELETED=0

# Clean scripts directory
echo "Cleaning scripts directory..."
for f in *.exe *.obj *.pdb *.ilk; do
    if [ -f "$f" ]; then
        echo "  Deleting: $f"
        rm -f "$f"
        FILES_DELETED=$((FILES_DELETED + 1))
    fi
done

# Clean root directory
echo "Cleaning root directory..."
cd ..
for f in *.exe *.obj *.pdb *.ilk; do
    if [ -f "$f" ]; then
        echo "  Deleting: $f"
        rm -f "$f"
        FILES_DELETED=$((FILES_DELETED + 1))
    fi
done

echo ""
if [ $FILES_DELETED -eq 0 ]; then
    echo "No build artifacts found"
else
    echo "Deleted $FILES_DELETED file(s)"
fi

echo ""
echo "Clean complete"
echo "============================================"

exit 0
