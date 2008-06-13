from pandac.PandaModules import Vec3
# Utility functions that are useful to both AI and client CartesianGrid code

class CartesianGridBase:
    def isValidZone(self, zoneId):
        def checkBounds(self=self, zoneId=zoneId):
            if ((zoneId < self.startingZone) or
                (zoneId > self.startingZone + self.gridSize * self.gridSize - 1)):
                return 0
            return 1
        if self.style == "Cartesian":
            return checkBounds()
        elif self.style == "CartesianStated":
            if zoneId >= 0 and zoneId < self.startingZone:
                return 1
            else:
                return checkBounds()
        else:
            return 0

    def getZoneFromXYZ(self, pos, wantRowAndCol=False):
        # NOTE: pos should be relative to our own grid origin
        # Convert a 3d position to a zone
        dx = self.cellWidth * self.gridSize * .5
        x = pos[0] + dx
        y = pos[1] + dx
        col = x // self.cellWidth
        row = y // self.cellWidth
        # Compute which zone we are in
        zoneId = int(self.startingZone + ((row * self.gridSize) + col))

        if (wantRowAndCol):
            return (zoneId,col,row)
        else:
            return zoneId

    def getGridSizeFromSphereRadius(self, sphereRadius, cellWidth, gridRadius):
        # NOTE: This ensures that the grid is at least a "gridRadius" number
        # of cells larger than the trigger sphere that loads the grid.  This
        # gives us some room to start setting interest to the grid before we
        # expect to see any objects on it.
        sphereRadius = max(sphereRadius, gridRadius*cellWidth)
        return 2 * (sphereRadius // cellWidth)

    def getGridSizeFromSphere(self, sphereRadius, spherePos, cellWidth, gridRadius):
        # NOTE: This ensures that the grid is at least a "gridRadius" number
        # of cells larger than the trigger sphere that loads the grid.  This
        # gives us some room to start setting interest to the grid before we
        # expect to see any objects on it.
        xMax = abs(spherePos[0])+sphereRadius
        yMax = abs(spherePos[1])+sphereRadius
        sphereRadius = Vec3(xMax,yMax,0).length()
        
        # sphereRadius = max(sphereRadius, gridRadius*cellWidth)
        return 2 * (sphereRadius // cellWidth)

    def getZoneCellOrigin(self, zoneId):
        # It returns the origin of the zoneCell
        # Origin is the top-left corner of zoneCell
        dx = self.cellWidth * self.gridSize * .5
        zone = zoneId - self.startingZone
        row = zone // self.gridSize
        col = zone % self.gridSize
        x = col * self.cellWidth - dx
        y = row * self.cellWidth - dx

        return (x, y, 0)

    def getZoneCellOriginCenter(self, zoneId):
        # Variant of the getZoneCellOrigin. It
        # returns the center of the zoneCell
        dx = self.cellWidth * self.gridSize * .5
        center = self.cellWidth * 0.5
        zone = zoneId - self.startingZone
        row = zone // self.gridSize
        col = zone % self.gridSize
        x = col * self.cellWidth - dx + center
        y = row * self.cellWidth - dx + center

        return (x, y, 0)

