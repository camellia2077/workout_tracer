import easyocr
import json
from PIL import Image
import numpy as np
import torch
import config  # 导入您的配置文件
import sys     # 导入 sys 模块以方便地退出程序

class OcrExtractor:
    """
    一个从图片指定区域提取文本的类。
    模型在创建实例时仅初始化一次，以提高效率。
    """
    def __init__(self):
        """
        初始化OcrExtractor，加载EasyOCR模型。
        如果可用，会自动检测并使用GPU。
        """
        use_gpu = torch.cuda.is_available()
        if use_gpu:
            print(f"检测到NVIDIA GPU: {torch.cuda.get_device_name(0)}。将使用 GPU 加速。")
        else:
            print("警告: 未检测到可用的NVIDIA GPU。将使用 CPU 运行。")
        
        self.reader = easyocr.Reader(['ch_sim', 'en'], gpu=use_gpu)
        print("模型加载完毕，提取器已准备就绪。")

    def extract_data_to_dict(self, image_path: str, fields: list) -> dict:
        """
        根据字段列表和坐标从图像中提取数据，并返回一个Python字典。

        :param image_path: 图片文件的路径。
        :param fields: 从配置文件中加载的字段列表。
        :return: 包含提取结果的Python字典。
        """
        results_dict = {}
        try:
            source_image = Image.open(image_path)
            print(f"\n图片 '{image_path}' 加载成功。")

            print("开始从以下字段提取数据:")
            for field in fields:
                field_name = field['name']
                box = field['box']
                
                print(f"  - 正在处理字段: '{field_name}'...")
                
                cropped_image_pil = source_image.crop(box)
                cropped_image_np = np.array(cropped_image_pil)
                ocr_results = self.reader.readtext(cropped_image_np)
                extracted_text = ' '.join([res[1] for res in ocr_results]).strip()
                results_dict[field_name] = extracted_text
            
            return results_dict

        except FileNotFoundError:
            return {"error": f"图片文件未找到: {image_path}"}
        except Exception as e:
            return {"error": f"处理过程中发生错误: {str(e)}"}

def sanitize_filename(name: str) -> str:
    """
    将一个字符串清理成一个安全的文件名。
    替换掉在Windows/Linux/Mac文件名中不安全的字符。
    """
    # 替换斜杠、冒号和空格
    name = name.replace('/', '-').replace(':', '-').replace(' ', '_')
    # 您可以在这里添加更多替换规则
    return name

# --- 主程序运行区 ---
if __name__ == "__main__":
    
    # 1. 创建提取器实例并执行提取
    extractor = OcrExtractor()
    extracted_data = extractor.extract_data_to_dict('running_data.jpg', config.FIELDS)
    
    # 2. 检查提取过程中是否发生错误
    if "error" in extracted_data:
        print(f"\n错误: {extracted_data['error']}")
        sys.exit(1) # 以错误码退出程序

    # 3. 检查关键字段 'start_time' 是否存在且有值
    start_time_value = extracted_data.get("start_time")
    
    if not start_time_value:
        # 如果 'start_time' 不存在或其值为空字符串
        print("\n" + "="*50)
        print("错误：关键字段 'start_time' 提取失败或为空。")
        print("无法生成文件名，程序将终止，不会输出JSON文件。")
        print("="*50)
        sys.exit(1) # 以错误码退出程序

    # 4. 如果一切正常，生成文件名并保存JSON文件
    try:
        # 清理 start_time 的值以用作文件名
        safe_filename = sanitize_filename(start_time_value) + ".json"
        
        # 使用 'with' 语句安全地写入文件
        with open(safe_filename, 'w', encoding='utf-8') as json_file:
            # indent=4 美化输出, ensure_ascii=False 保证中文正常显示
            json.dump(extracted_data, json_file, indent=4, ensure_ascii=False)
        
        print("\n" + "="*40)
        print("提取完成!")
        print(f"数据已成功保存到文件: {safe_filename}")
        print("="*40)

    except Exception as e:
        print(f"\n在写入JSON文件时发生错误: {e}")
        sys.exit(1)