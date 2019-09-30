from direct.directnotify.DirectNotifyGlobal import *
from panda3d.core import PNMImage, Filename, PNMFileTypeRegistry, StringStream
import struct


class Icon:
    """ This class is used to create an icon for various platforms. """
    notify = directNotify.newCategory("Icon")

    def __init__(self):
        self.images = {}

    def addImage(self, image):
        """ Adds an image to the icon.  Returns False on failure, True on success.
        Only one image per size can be loaded, and the image size must be square. """

        if not isinstance(image, PNMImage):
            fn = image
            if not isinstance(fn, Filename):
                fn = Filename.fromOsSpecific(fn)

            image = PNMImage()
            if not image.read(fn):
                Icon.notify.warning("Image '%s' could not be read" % fn.getBasename())
                return False

        if image.getXSize() != image.getYSize():
            Icon.notify.warning("Ignoring image without square size")
            return False

        self.images[image.getXSize()] = image

        return True

    def generateMissingImages(self):
        """ Generates image sizes that should be present but aren't by scaling
        from the next higher size. """

        for required_size in (256, 128, 48, 32, 16):
            if required_size in self.images:
                continue

            sizes = sorted(self.images.keys())
            if required_size * 2 in sizes:
                from_size = required_size * 2
            else:
                from_size = 0
                for from_size in sizes:
                    if from_size > required_size:
                        break

            if from_size > required_size:
                Icon.notify.warning("Generating %dx%d icon by scaling down %dx%d image" % (required_size, required_size, from_size, from_size))

                image = PNMImage(required_size, required_size)
                if self.images[from_size].hasAlpha():
                    image.addAlpha()
                image.quickFilterFrom(self.images[from_size])
                self.images[required_size] = image
            else:
                Icon.notify.warning("Cannot generate %dx%d icon; no higher resolution image available" % (required_size, required_size))

    def _write_bitmap(self, fp, image, size, bpp):
        """ Writes the bitmap header and data of an .ico file. """

        fp.write(struct.pack('<IiiHHIIiiII', 40, size, size * 2, 1, bpp, 0, 0, 0, 0, 0, 0))

        # XOR mask
        if bpp == 24:
            # Align rows to 4-byte boundary
            rowalign = b'\0' * (-(size * 3) & 3)
            for y in range(size):
                for x in range(size):
                    r, g, b = image.getXel(x, size - y - 1)
                    fp.write(struct.pack('<BBB', int(b * 255), int(g * 255), int(r * 255)))
                fp.write(rowalign)

        elif bpp == 32:
            for y in range(size):
                for x in range(size):
                    r, g, b, a = image.getXelA(x, size - y - 1)
                    fp.write(struct.pack('<BBBB', int(b * 255), int(g * 255), int(r * 255), int(a * 255)))

        elif bpp == 8:
            # We'll have to generate a palette of 256 colors.
            hist = PNMImage.Histogram()
            image2 = PNMImage(image)
            if image2.hasAlpha():
                image2.premultiplyAlpha()
                image2.removeAlpha()
            image2.quantize(256)
            image2.make_histogram(hist)
            colors = list(hist.get_pixels())
            assert len(colors) <= 256

            # Write the palette.
            i = 0
            while i < 256 and i < len(colors):
                r, g, b, a = colors[i]
                fp.write(struct.pack('<BBBB', b, g, r, 0))
                i += 1
            if i < 256:
                # Fill the rest with zeroes.
                fp.write(b'\x00' * (4 * (256 - i)))

            # Write indices.  Align rows to 4-byte boundary.
            rowalign = b'\0' * (-size & 3)
            for y in range(size):
                for x in range(size):
                    pixel = image2.get_pixel(x, size - y - 1)
                    index = colors.index(pixel)
                    if index >= 256:
                        # Find closest pixel instead.
                        index = closest_indices[index - 256]
                    fp.write(struct.pack('<B', index))
                fp.write(rowalign)
        else:
            raise ValueError("Invalid bpp %d" % (bpp))

        # Create an AND mask, aligned to 4-byte boundary
        if image.hasAlpha() and bpp <= 8:
            rowalign = b'\0' * (-((size + 7) >> 3) & 3)
            for y in range(size):
                mask = 0
                num_bits = 7
                for x in range(size):
                    a = image.get_alpha_val(x, size - y - 1)
                    if a <= 1:
                        mask |= (1 << num_bits)
                    num_bits -= 1
                    if num_bits < 0:
                        fp.write(struct.pack('<B', mask))
                        mask = 0
                        num_bits = 7
                if num_bits < 7:
                    fp.write(struct.pack('<B', mask))
                fp.write(rowalign)
        else:
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            fp.write(b'\x00' * (andsize * size))

    def makeICO(self, fn):
        """ Writes the images to a Windows ICO file.  Returns True on success. """

        if not isinstance(fn, Filename):
            fn = Filename.fromOsSpecific(fn)
        fn.setBinary()

        # ICO files only support resolutions up to 256x256.
        count = 0
        for size in self.images.keys():
            if size < 256:
                count += 1
            if size <= 256:
                count += 1
        dataoffs = 6 + count * 16

        ico = open(fn, 'wb')
        ico.write(struct.pack('<HHH', 0, 1, count))

        # Write 8-bpp image headers for sizes under 256x256.
        for size, image in self.images.items():
            if size >= 256:
                continue
            ico.write(struct.pack('<BB', size, size))

            # Calculate row sizes
            xorsize = size
            if xorsize % 4 != 0:
                xorsize += 4 - (xorsize % 4)
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            datasize = 40 + 256 * 4 + (xorsize + andsize) * size

            ico.write(struct.pack('<BBHHII', 0, 0, 1, 8, datasize, dataoffs))
            dataoffs += datasize

        # Write 24/32-bpp image headers.
        for size, image in self.images.items():
            if size > 256:
                continue
            elif size == 256:
                ico.write(b'\0\0')
            else:
                ico.write(struct.pack('<BB', size, size))

            # Calculate the size so we can write the offset within the file.
            if image.hasAlpha():
                bpp = 32
                xorsize = size * 4
            else:
                bpp = 24
                xorsize = size * 3 + (-(size * 3) & 3)
            andsize = (size + 7) >> 3
            if andsize % 4 != 0:
                andsize += 4 - (andsize % 4)
            datasize = 40 + (xorsize + andsize) * size

            ico.write(struct.pack('<BBHHII', 0, 0, 1, bpp, datasize, dataoffs))
            dataoffs += datasize

        # Now write the actual icon bitmap data.
        for size, image in self.images.items():
            if size < 256:
                self._write_bitmap(ico, image, size, 8)

        for size, image in self.images.items():
            if size <= 256:
                bpp = 32 if image.hasAlpha() else 24
                self._write_bitmap(ico, image, size, bpp)

        assert ico.tell() == dataoffs
        ico.close()

        return True

    def makeICNS(self, fn):
        """ Writes the images to an Apple ICNS file.  Returns True on success. """

        if not isinstance(fn, Filename):
            fn = Filename.fromOsSpecific(fn)
        fn.setBinary()

        icns = open(fn, 'wb')
        icns.write(b'icns\0\0\0\0')

        icon_types = {16: b'is32', 32: b'il32', 48: b'ih32', 128: b'it32'}
        mask_types = {16: b's8mk', 32: b'l8mk', 48: b'h8mk', 128: b't8mk'}
        png_types = {256: b'ic08', 512: b'ic09', 1024: b'ic10'}

        pngtype = PNMFileTypeRegistry.getGlobalPtr().getTypeFromExtension("png")

        for size, image in sorted(self.images.items(), key=lambda item:item[0]):
            if size in png_types and pngtype is not None:
                stream = StringStream()
                image.write(stream, "", pngtype)
                pngdata = stream.data

                icns.write(png_types[size])
                icns.write(struct.pack('>I', len(pngdata)))
                icns.write(pngdata)

            elif size in icon_types:
                # If it has an alpha channel, we write out a mask too.
                if image.hasAlpha():
                    icns.write(mask_types[size])
                    icns.write(struct.pack('>I', size * size + 8))

                    for y in range(size):
                        for x in range(size):
                            icns.write(struct.pack('<B', int(image.getAlpha(x, y) * 255)))

                icns.write(icon_types[size])
                icns.write(struct.pack('>I', size * size * 4 + 8))

                for y in range(size):
                    for x in range(size):
                        r, g, b = image.getXel(x, y)
                        icns.write(struct.pack('>BBBB', 0, int(r * 255), int(g * 255), int(b * 255)))

        length = icns.tell()
        icns.seek(4)
        icns.write(struct.pack('>I', length))
        icns.close()

        return True
