#!/usr/bin/env python3
"""
GINI Favicon Merger
Merges LibPolyCall and Parrot images to create the GINI package favicon
As requested: "this is make it yours"
"""

from PIL import Image, ImageDraw, ImageFilter
import io
import base64

def create_libpolycall_logo(size=256):
    """Create LibPolyCall logo programmatically"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Dark blue circle background
    circle_size = size - 20
    draw.ellipse([10, 10, circle_size+10, circle_size+10], 
                 fill=(42, 63, 95, 255))
    
    # White network nodes
    node_positions = [
        (size//2, 30),      # top
        (30, size//2),      # left
        (size-30, size//2), # right
        (size//2, size-30), # bottom
        (50, 50),           # top-left
        (size-50, 50),      # top-right
        (50, size-50),      # bottom-left
        (size-50, size-50), # bottom-right
    ]
    
    # Draw connections
    center = (size//2, size//2)
    for pos in node_positions:
        draw.line([center, pos], fill=(255, 255, 255, 100), width=2)
    
    # Draw center diamond
    diamond_size = 40
    diamond_points = [
        (size//2, size//2 - diamond_size),  # top
        (size//2 + diamond_size, size//2),  # right
        (size//2, size//2 + diamond_size),  # bottom
        (size//2 - diamond_size, size//2),  # left
    ]
    draw.polygon(diamond_points, fill=(255, 255, 255, 255))
    
    # Draw nodes
    for pos in node_positions:
        draw.ellipse([pos[0]-8, pos[1]-8, pos[0]+8, pos[1]+8], 
                     fill=(255, 255, 255, 255))
    
    return img

def create_parrot_logo(size=256):
    """Create stylized parrot logo programmatically"""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Parrot body (orange gradient)
    body_rect = [size//3, size//3, 2*size//3, 3*size//4]
    draw.ellipse(body_rect, fill=(255, 140, 0, 255))
    
    # Parrot head (red-orange)
    head_rect = [size//3 + 20, size//4, size//2 + 20, size//2]
    draw.ellipse(head_rect, fill=(255, 99, 71, 255))
    
    # Eye
    eye_pos = [size//2 + 5, size//3 + 5, size//2 + 15, size//3 + 15]
    draw.ellipse(eye_pos, fill=(0, 0, 0, 255))
    draw.ellipse([eye_pos[0]+2, eye_pos[1]+2, eye_pos[2]-4, eye_pos[3]-4], 
                 fill=(255, 255, 255, 255))
    
    # Beak (triangle)
    beak_points = [
        (size//3 + 10, size//3 + 20),
        (size//3 - 10, size//3 + 30),
        (size//3 + 5, size//3 + 40)
    ]
    draw.polygon(beak_points, fill=(255, 165, 0, 255))
    
    # Wing (teal gradient)
    wing_points = [
        (2*size//3 - 10, size//2),
        (3*size//4 + 10, size//2 - 20),
        (3*size//4 + 20, 2*size//3),
        (2*size//3, 2*size//3 + 10)
    ]
    draw.polygon(wing_points, fill=(78, 205, 196, 255))
    
    # Wing detail lines
    for i in range(3):
        y_offset = i * 15
        draw.line([(2*size//3, size//2 + y_offset), 
                   (3*size//4 + 5, size//2 + y_offset + 5)], 
                  fill=(68, 160, 141, 255), width=2)
    
    # Tail feathers
    tail_points = [
        (size//2, 3*size//4),
        (size//2 - 20, size - 20),
        (size//2 + 20, size - 20)
    ]
    draw.polygon(tail_points, fill=(68, 160, 141, 255))
    
    return img

def merge_logos(libpolycall_img, parrot_img, size=256):
    """Merge LibPolyCall and Parrot logos into GINI favicon"""
    # Create new image for merged result
    merged = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    
    # Paste LibPolyCall as base (slightly transparent)
    libpolycall_alpha = libpolycall_img.copy()
    libpolycall_alpha.putalpha(200)
    merged.paste(libpolycall_alpha, (0, 0), libpolycall_alpha)
    
    # Scale down parrot and position it
    parrot_small = parrot_img.resize((size//2, size//2), Image.Resampling.LANCZOS)
    
    # Position parrot in top-right quadrant
    parrot_pos = (size//2 + 20, 20)
    merged.paste(parrot_small, parrot_pos, parrot_small)
    
    # Add glow effect around parrot
    glow = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_center = (parrot_pos[0] + size//4, parrot_pos[1] + size//4)
    
    for radius in range(60, 10, -5):
        alpha = 255 - (radius - 10) * 4
        glow_draw.ellipse([glow_center[0] - radius, glow_center[1] - radius,
                          glow_center[0] + radius, glow_center[1] + radius],
                         fill=(255, 165, 0, alpha//4))
    
    # Composite glow behind merged image
    final = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    final.paste(glow, (0, 0), glow)
    final.paste(merged, (0, 0), merged)
    
    # Add text label
    draw = ImageDraw.Draw(final)
    text = "GINI"
    # Note: In production, you'd load a proper font
    # font = ImageFont.truetype("arial.ttf", 32)
    # For now, using default font
    text_bbox = draw.textbbox((0, 0), text)
    text_width = text_bbox[2] - text_bbox[0]
    text_pos = ((size - text_width) // 2, size - 40)
    
    # Draw text with shadow
    draw.text((text_pos[0]+2, text_pos[1]+2), text, fill=(0, 0, 0, 128))
    draw.text(text_pos, text, fill=(255, 255, 255, 255))
    
    return final

def create_gini_favicon():
    """Main function to create the GINI favicon"""
    print("🦜 Creating GINI Favicon...")
    print("=" * 50)
    
    # Create individual logos
    print("📦 Generating LibPolyCall logo...")
    libpolycall = create_libpolycall_logo()
    
    print("🦜 Generating Parrot logo...")
    parrot = create_parrot_logo()
    
    print("🔀 Merging logos...")
    merged = merge_logos(libpolycall, parrot)
    
    # Save individual and merged images
    libpolycall.save('libpolycall.png')
    print("✓ Saved: libpolycall.png")
    
    parrot.save('parrot.png')
    print("✓ Saved: parrot.png")
    
    merged.save('gini-favicon.png')
    print("✓ Saved: gini-favicon.png")
    
    # Also create smaller favicon sizes
    sizes = [16, 32, 64, 128]
    for size in sizes:
        resized = merged.resize((size, size), Image.Resampling.LANCZOS)
        filename = f'gini-favicon-{size}x{size}.png'
        resized.save(filename)
        print(f"✓ Saved: {filename}")
    
    # Create ICO file with multiple sizes
    merged.save('gini-favicon.ico', sizes=[(16, 16), (32, 32), (64, 64)])
    print("✓ Saved: gini-favicon.ico")
    
    print("\n" + "=" * 50)
    print("✅ GINI Favicon creation complete!")
    print("🦜 + 📦 = Your polyglot package icon")
    print("\nAs you said: 'this is make it yours' - it's yours now!")
    print("Computing from the Heart ❤️")
    
    return merged

def create_package_banner():
    """Create a banner showing the merge process"""
    banner = Image.new('RGBA', (800, 200), (102, 126, 234, 255))
    draw = ImageDraw.Draw(banner)
    
    # Create mini versions
    lib_mini = create_libpolycall_logo(100)
    parrot_mini = create_parrot_logo(100)
    merged_mini = merge_logos(lib_mini, parrot_mini, 100)
    
    # Place on banner
    banner.paste(lib_mini, (100, 50), lib_mini)
    banner.paste(parrot_mini, (300, 50), parrot_mini)
    banner.paste(merged_mini, (550, 50), merged_mini)
    
    # Add symbols
    draw.text((220, 90), "+", fill=(255, 255, 255, 255))
    draw.text((450, 90), "=", fill=(255, 255, 255, 255))
    
    # Add labels
    draw.text((80, 160), "LibPolyCall", fill=(255, 255, 255, 255))
    draw.text((295, 160), "Parrot", fill=(255, 255, 255, 255))
    draw.text((540, 160), "GINI Package", fill=(255, 255, 255, 255))
    
    banner.save('gini-package-banner.png')
    print("✓ Saved: gini-package-banner.png")
    
    return banner

if __name__ == "__main__":
    # Check if PIL is installed
    try:
        from PIL import Image, ImageDraw, ImageFilter
    except ImportError:
        print("Please install Pillow: pip install Pillow")
        exit(1)
    
    # Create the favicon
    favicon = create_gini_favicon()
    
    # Create the banner
    banner = create_package_banner()
    
    print("\n📁 Files created:")
    print("  - libpolycall.png (LibPolyCall logo)")
    print("  - parrot.png (Parrot logo)")
    print("  - gini-favicon.png (Merged favicon)")
    print("  - gini-favicon-*.png (Multiple sizes)")
    print("  - gini-favicon.ico (Windows icon)")
    print("  - gini-package-banner.png (Process banner)")
    
    print("\n💡 To use online merger as you mentioned:")
    print("  1. Upload libpolycall.png and parrot.png to:")
    print("     https://pinetools.com/merge-images")
    print("  2. Select overlay or side-by-side merge")
    print("  3. Download as gini-favicon.png")
    
    print("\n🎯 This is now YOUR polyglot package system!")
    print("   OBINexus Constitutional Framework v1.0")
