
# Utility functions that are useful to both AI and client CartesianGrid code

class CartesianGridBase:
    def isValidZone(self, zoneId):
        if self.style == "Cartesian":
            if ((zoneId < self.startingZone) or
                (zoneId > self.startingZone + self.gridSize * self.gridSize - 1)):
                return 0
            return 1
        elif self.style == "CartesianStated":
            return 1
        else:
            return 0
    
    def getZoneFromXYZ(self, pos):
        # NOTE: pos should be relative to our own grid origin
        # Convert a 3d position to a zone
        dx = self.cellWidth * self.gridSize * .5
        x = pos[0] + dx
        y = pos[1] + dx
        col = x // self.cellWidth
        row = y // self.cellWidth
        # Compute which zone we are in
        zoneId = int(self.startingZone + ((row * self.gridSize) + col))

        return zoneId

    def getGridSizeFromSphereRadius(self, sphereRadius, cellWidth, gridRadius):
        # NOTE: This ensures that the grid is at least a "gridRadius" number of cells
        # larger than the trigger sphere that loads the grid.  This gives us some
        # room to start setting interest to the grid before we expect to see any objects
        # on it.
        return 2 * (sphereRadius // cellWidth)
        
    def getZoneCellOrigin(self, zoneId):
        dx = self.cellWidth * self.gridSize * .5
        zone = zoneId - self.startingZone
        row = zone // self.gridSize
        col = zone % self.gridSize
        x = col * self.cellWidth - dx
        y = row * self.cellWidth - dx

        return (x,y,0)
    
