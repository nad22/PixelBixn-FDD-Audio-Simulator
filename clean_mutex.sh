#!/bin/bash
# Script to remove all mutex calls from main.cpp
sed -i 's/xSemaphoreTake.*;//g' src/main.cpp
sed -i 's/xSemaphoreGive.*;//g' src/main.cpp
sed -i '/fileSystemMutex/d' src/main.cpp